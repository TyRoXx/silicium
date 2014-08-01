#include <reactive/received_from_socket_source.hpp>
#include <reactive/sending_observable.hpp>
#include <reactive/flatten.hpp>
#include <reactive/observable_source.hpp>
#include <reactive/tcp_acceptor.hpp>
#include <reactive/coroutine.hpp>
#include <silicium/http/http.hpp>
#include <reactive/total_consumer.hpp>
#include <boost/format.hpp>
#include <boost/thread/future.hpp>

namespace
{
	template <class Socket>
	void serve_client(
		Socket &client,
		boost::uintmax_t visitor_number)
	{
		rx::received_from_socket_source bytes_receiver(client.receiving());
		boost::optional<Si::http::request_header> request = Si::http::parse_header(bytes_receiver);
		if (!request)
		{
			return;
		}
		std::vector<char> send_buffer;
		{
			auto response_sink = Si::make_container_sink(send_buffer);
			Si::http::response_header response;
			response.http_version = "HTTP/1.0";
			response.status = 200;
			response.status_text = "OK";
			std::string const body = boost::str(boost::format("Hello, visitor %1%") % visitor_number);
			response.arguments["Content-Length"] = boost::lexical_cast<std::string>(body.size());
			response.arguments["Connection"] = "close";
			Si::http::write_header(response_sink, response);
			Si::append(response_sink, body);
		}
		client.send(boost::make_iterator_range(send_buffer.data(), send_buffer.data() + send_buffer.size()));
		client.shutdown();

		while (Si::get(bytes_receiver))
		{
		}
	}

	struct coroutine_socket
	{
		explicit coroutine_socket(boost::asio::ip::tcp::socket &socket, rx::yield_context<rx::detail::nothing> &yield)
			: socket(&socket)
			, yield(&yield)
			, received(4096)
			, receiver(socket, boost::make_iterator_range(received.data(), received.data() + received.size()))
			, receiving_(receiver, yield)
		{
		}

		Si::source<rx::received_from_socket> &receiving()
		{
			return receiving_;
		}

		void send(boost::iterator_range<char const *> data)
		{
			assert(socket);
			assert(yield);

			rx::sending_observable sending(*socket, data);

			//ignore error
			yield->get_one(sending);
		}

		void shutdown()
		{
			boost::system::error_code error;
			socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
		}

	private:

		boost::asio::ip::tcp::socket *socket = nullptr;
		rx::yield_context<rx::detail::nothing> *yield = nullptr;
		std::vector<char> received;
		rx::socket_observable receiver;
		rx::observable_source<rx::socket_observable, rx::yield_context<rx::detail::nothing>> receiving_;
	};

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
				coroutine_socket coro_socket(*client, yield);
				return serve_client(coro_socket, visitor_number_);
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
		boost::uintmax_t visitor_count = 0;
		for (;;)
		{
			auto result = yield.get_one(clients);
			if (!result)
			{
				break;
			}
			++visitor_count;
			accept_handler handler{visitor_count};
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
