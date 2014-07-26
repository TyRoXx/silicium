#include <reactive/received_from_socket_source.hpp>
#include <reactive/sending_observable.hpp>
#include <reactive/flatten.hpp>
#include <reactive/socket_observable.hpp>
#include <reactive/observable_source.hpp>
#include <reactive/tcp_acceptor.hpp>
#include <reactive/coroutine.hpp>
#include <silicium/http/http.hpp>
#include <silicium/fast_variant.hpp>
#include <reactive/finite_state_machine.hpp>
#include <reactive/generate.hpp>
#include <reactive/total_consumer.hpp>
#include <boost/asio.hpp>
#include <boost/ref.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/format.hpp>
#include <boost/interprocess/sync/null_mutex.hpp>
#include <boost/thread/future.hpp>

namespace
{
	void serve_client(boost::asio::ip::tcp::socket &client, rx::yield_context<rx::detail::nothing> &yield, boost::uintmax_t visitor_number)
	{
		std::vector<char> received(4096);
		rx::socket_observable receiving(client, boost::make_iterator_range(received.data(), received.data() + received.size()));
		auto receiver = rx::make_observable_source(rx::ref(receiving), yield);
		rx::received_from_socket_source bytes_receiver(receiver);
		boost::optional<Si::http::request_header> request = Si::http::parse_header(bytes_receiver);
		if (!request)
		{
			return;
		}
		std::vector<char> send_buffer;
		{
			auto response_sink = Si::make_container_sink(send_buffer);
			Si::http::response_header response;
			response.http_version = "HTTP/1.1";
			response.status = 200;
			response.status_text = "OK";
			std::string const body = boost::str(boost::format("Hello, visitor %1%") % visitor_number);
			response.arguments["Content-Length"] = boost::lexical_cast<std::string>(body.size());
			response.arguments["Connection"] = "close";
			Si::http::write_header(response_sink, response);
			Si::append(response_sink, body);
		}
		rx::sending_observable sending(
			client,
			boost::make_iterator_range(send_buffer.data(), send_buffer.data() + send_buffer.size()));

		//ignore error
		yield.get_one(sending);

		boost::system::error_code error;
		client.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);

		while (Si::get(bytes_receiver))
		{
		}
	}

	using events = rx::shared_observable<rx::detail::nothing>;

	struct accept_handler : boost::static_visitor<events>
	{
		boost::uintmax_t visitor_number;

		explicit accept_handler(boost::uintmax_t visitor_number)
			: visitor_number(visitor_number)
		{
		}

		events operator()(std::shared_ptr<boost::asio::ip::tcp::socket> client) const
		{
			auto visitor_number_ = visitor_number;
			auto client_handler = rx::wrap<rx::detail::nothing>(rx::make_coroutine<rx::detail::nothing>([client, visitor_number_](rx::yield_context<rx::detail::nothing> &yield) -> void
			{
				return serve_client(*client, yield, visitor_number_);
			}));
			return client_handler;
		}

		events operator()(boost::system::error_code) const
		{
			throw std::logic_error("not implemented");
		}
	};
}

int main()
{
	boost::asio::io_service io;
	boost::asio::ip::tcp::acceptor acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(), 8080));
	rx::tcp_acceptor clients(acceptor);
	auto handling_clients = rx::flatten<boost::mutex>(rx::make_coroutine<events>([&clients](rx::yield_context<events> &yield) -> void
	{
		auto visitor_counter = rx::make_finite_state_machine(
									rx::generate([]() { return rx::nothing{}; }),
									static_cast<boost::uintmax_t>(0),
									[](boost::uintmax_t previous, rx::nothing) { return previous + 1; });
		for (;;)
		{
			auto result = yield.get_one(clients);
			if (!result)
			{
				break;
			}
			auto const visitor_number = yield.get_one(visitor_counter);
			assert(visitor_number);
			accept_handler handler{*visitor_number};
			auto context = Si::apply_visitor(handler, *result);
			if (!context.empty())
			{
				yield(std::move(context));
			}
		}
	}));
	auto all = rx::make_total_consumer(rx::ref(handling_clients));
	all.start();

	auto const thread_count = boost::thread::hardware_concurrency();

	std::vector<boost::unique_future<void>> workers;
	std::generate_n(std::back_inserter(workers), thread_count - 1, [&io]
	{
		return boost::async(boost::launch::async, [&io]
		{
			io.run();
		});
	});
	std::for_each(workers.begin(), workers.end(), [](boost::unique_future<void> &worker)
	{
		worker.get();
	});
	io.run();
}
