#include <silicium/source/received_from_socket_source.hpp>
#include <silicium/observable/flatten.hpp>
#include <silicium/asio/writing_observable.hpp>
#include <silicium/source/observable_source.hpp>
#include <silicium/asio/tcp_acceptor.hpp>
#include <silicium/observable/coroutine.hpp>
#include <silicium/observable/constant.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <silicium/http/http.hpp>
#include <silicium/observable/virtualized.hpp>
#include <silicium/source/virtualized_source.hpp>
#include <silicium/observable/total_consumer.hpp>
#include <silicium/observable/coroutine_generator.hpp>
#include <silicium/observable/erase_shared.hpp>
#include <silicium/observable/erase_unique.hpp>
#include <silicium/asio/reading_observable.hpp>
#include <silicium/terminate_on_exception.hpp>
#include <boost/format.hpp>
#include <boost/thread/future.hpp>
#include <cassert>

#if SILICIUM_HAS_EXCEPTIONS
#include <thread>

namespace
{
	template <class Socket>
	void serve_client(
		Socket &client,
		boost::uintmax_t visitor_number)
	{
		Si::received_from_socket_source bytes_receiver(client.receiving());
		Si::optional<Si::http::request> request = Si::http::parse_request(bytes_receiver);
		if (!request)
		{
			return;
		}
		std::vector<char> send_buffer;
		{
			auto response_sink = Si::make_container_sink(send_buffer);
			Si::http::response response;
			response.arguments = Si::make_unique<std::map<Si::noexcept_string, Si::noexcept_string>>();
			response.http_version = "HTTP/1.0";
			response.status = 200;
			response.status_text = "OK";
			std::string const body = boost::str(boost::format("Hello, visitor %1%") % visitor_number);
			(*response.arguments)["Content-Length"] = boost::lexical_cast<Si::noexcept_string>(body.size());
			(*response.arguments)["Connection"] = "close";
			Si::http::generate_response(response_sink, response);
			Si::append(response_sink, body);
		}
		client.send(Si::make_iterator_range(send_buffer.data(), send_buffer.data() + send_buffer.size()));
		client.shutdown();

		while (Si::get(bytes_receiver))
		{
		}
	}

	typedef Si::asio::reading_observable<boost::asio::ip::tcp::socket> socket_observable;

	struct coroutine_socket
	{
		explicit coroutine_socket(boost::asio::ip::tcp::socket &socket, Si::push_context<Si::nothing> &yield)
			: socket(&socket)
			, yield(&yield)
			, received(4096)
			, receiving_(Si::observable_source<socket_observable, Si::push_context<Si::nothing>>(socket_observable(socket, Si::make_iterator_range(received.data(), received.data() + received.size())), yield))
		{
		}

		Si::Source<Si::error_or<Si::memory_range>>::interface &receiving()
		{
			return receiving_;
		}

		void send(Si::iterator_range<char const *> data)
		{
			assert(socket);
			assert(yield);

			auto sending = Si::asio::make_writing_observable(*socket);
			sending.set_buffer(data);
			auto virtualized = Si::virtualize_observable<Si::ptr_observer<Si::observer<boost::system::error_code>>>(sending);

			//ignore error
			yield->get_one(virtualized);
		}

		void shutdown()
		{
			boost::system::error_code error;
			socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
		}

	private:

		boost::asio::ip::tcp::socket *socket;
		Si::push_context<Si::nothing> *yield;
		std::vector<char> received;
		Si::virtualized_source<Si::observable_source<socket_observable, Si::push_context<Si::nothing>>> receiving_;
	};

	struct thread_socket_source : Si::Source<Si::error_or<Si::memory_range>>::interface
	{
		typedef Si::error_or<Si::memory_range> element_type;

		explicit thread_socket_source(boost::asio::ip::tcp::socket &socket)
			: socket(&socket)
			, received(4096)
		{
		}

		boost::asio::ip::tcp::socket *get_socket() const
		{
			return socket;
		}

		virtual Si::iterator_range<element_type const *> map_next(std::size_t size) SILICIUM_OVERRIDE
		{
			(void)size;
			return {};
		}

		virtual element_type *copy_next(Si::iterator_range<element_type *> destination) SILICIUM_OVERRIDE
		{
			assert(socket);
			auto *i = destination.begin();
			if (i != destination.end())
			{
				boost::system::error_code ec;
				auto const bytes_received = socket->receive(boost::asio::buffer(received), 0, ec);
				if (ec)
				{
					*i = ec;
				}
				else
				{
					*i = Si::make_memory_range<char const>(received.data(), received.data() + bytes_received);
				}
				++i;
			}
			return i;
		}

	private:

		boost::asio::ip::tcp::socket *socket;
		std::vector<char> received;
	};

	struct thread_socket
	{
		explicit thread_socket(boost::asio::ip::tcp::socket &socket)
			: receiving_(socket)
		{
		}

		Si::Source<Si::error_or<Si::memory_range>>::interface &receiving()
		{
			return receiving_;
		}

		void send(Si::iterator_range<char const *> data)
		{
			assert(receiving_.get_socket());
			boost::asio::write(*receiving_.get_socket(), boost::asio::buffer(data.begin(), data.size()));
		}

		void shutdown()
		{
			assert(receiving_.get_socket());
			boost::system::error_code error;
			receiving_.get_socket()->shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
		}

	private:

		thread_socket_source receiving_;
	};

	typedef Si::shared_observable<Si::nothing> events;

#if SILICIUM_HAS_COROUTINE_GENERATOR
	struct coroutine_web_server
	{
		explicit coroutine_web_server(boost::asio::io_service &io, boost::uint16_t port)
			: acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(), port))
			, clients(&acceptor)
			, all_work(Si::make_total_consumer(Si::erase_unique(Si::flatten<boost::mutex>(Si::make_coroutine_generator<events>([this](Si::push_context<events> &yield) -> void
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
					std::shared_ptr<boost::asio::ip::tcp::socket> client = result->get();
					auto context =
						[visitor_count, client]() -> events
					{
						auto visitor_number_ = visitor_count;
						auto client_handler = Si::erase_shared(Si::make_coroutine_generator<Si::nothing>([client, visitor_number_](Si::push_context<Si::nothing> &yield) -> void
						{
							coroutine_socket coro_socket(*client, yield);
							return serve_client(coro_socket, visitor_number_);
						}));
						return client_handler;
					}();
					if (!context.empty())
					{
						yield(std::move(context));
					}
				}
			})))))
		{
		}

		void start()
		{
			all_work.start();
		}

	private:

		boost::asio::ip::tcp::acceptor acceptor;
		Si::asio::tcp_acceptor<boost::asio::ip::tcp::acceptor *> clients;
		Si::total_consumer<Si::unique_observable<Si::nothing>> all_work;
	};

	struct thread_web_server
	{
		explicit thread_web_server(boost::asio::io_service &io, boost::uint16_t port)
			: acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(), port))
			, clients(&acceptor)
			, all_work(Si::make_total_consumer(Si::erase_unique(Si::flatten<boost::mutex>(Si::make_coroutine_generator<events>([this](Si::push_context<events> &yield) -> void
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
					std::shared_ptr<boost::asio::ip::tcp::socket> client = result->get();
					std::thread([client, visitor_count]
					{
						thread_socket threaded_socket(*client);
						serve_client(threaded_socket, visitor_count);
					}).detach();
				}
			})))))
		{
		}

		void start()
		{
			all_work.start();
		}

	private:

		boost::asio::ip::tcp::acceptor acceptor;
		Si::asio::tcp_acceptor<boost::asio::ip::tcp::acceptor *> clients;
		Si::total_consumer<Si::unique_observable<Si::nothing>> all_work;
	};
#endif
}
#endif

int main()
{
	boost::asio::io_service io;

#if SILICIUM_HAS_COROUTINE_GENERATOR
	coroutine_web_server coroutined(io, 8080);
	coroutined.start();

	thread_web_server threaded(io, 8081);
	threaded.start();

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
#else
	std::cerr << "This example requires coroutine support\n";
#endif
	io.run();
}
