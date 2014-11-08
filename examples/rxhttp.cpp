#include <silicium/source/received_from_socket_source.hpp>
#include <silicium/asio/sending_observable.hpp>
#include <silicium/flatten.hpp>
#include <silicium/source/observable_source.hpp>
#include <silicium/asio/tcp_acceptor.hpp>
#include <silicium/coroutine.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <silicium/http/http.hpp>
#include <silicium/virtualized_observable.hpp>
#include <silicium/source/virtualized_source.hpp>
#include <silicium/total_consumer.hpp>
#include <silicium/coroutine_generator.hpp>
#include <boost/format.hpp>
#include <boost/thread/future.hpp>
#include <thread>
#include <cassert>

namespace
{
	template <class Socket>
	void serve_client(
		Socket &client,
		boost::uintmax_t visitor_number)
	{
		Si::received_from_socket_source bytes_receiver(client.receiving());
		boost::optional<Si::http::request_header> request = Si::http::parse_header(bytes_receiver);
		if (!request)
		{
			return;
		}
		std::vector<char> send_buffer;
		{
			auto response_sink = Si::make_container_sink(send_buffer);
			Si::http::response_header response;
			response.arguments = Si::make_unique<std::map<Si::noexcept_string, Si::noexcept_string>>();
			response.http_version = "HTTP/1.0";
			response.status = 200;
			response.status_text = "OK";
			std::string const body = boost::str(boost::format("Hello, visitor %1%") % visitor_number);
			(*response.arguments)["Content-Length"] = boost::lexical_cast<Si::noexcept_string>(body.size());
			(*response.arguments)["Connection"] = "close";
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
		explicit coroutine_socket(boost::asio::ip::tcp::socket &socket, Si::push_context<Si::nothing> &yield)
			: socket(&socket)
			, yield(&yield)
			, received(4096)
			, receiver(socket, boost::make_iterator_range(received.data(), received.data() + received.size()))
			, receiving_(Si::observable_source<Si::socket_observable, Si::push_context<Si::nothing>>(receiver, yield))
		{
		}

		Si::source<Si::received_from_socket> &receiving()
		{
			return receiving_;
		}

		void send(boost::iterator_range<char const *> data)
		{
			assert(socket);
			assert(yield);

			auto sending = Si::virtualize_observable(Si::sending_observable(*socket, data));

			//ignore error
			yield->get_one(sending);
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
		Si::socket_observable receiver;
		Si::virtualized_source<Si::observable_source<Si::socket_observable, Si::push_context<Si::nothing>>> receiving_;
	};

	struct thread_socket_source : Si::source<Si::received_from_socket>
	{
		typedef Si::received_from_socket element_type;

		explicit thread_socket_source(boost::asio::ip::tcp::socket &socket)
			: socket(&socket)
			, received(4096)
		{
		}

		boost::asio::ip::tcp::socket *get_socket() const
		{
			return socket;
		}

		virtual boost::iterator_range<element_type const *> map_next(std::size_t size) SILICIUM_OVERRIDE
		{
			(void)size;
			return {};
		}

		virtual element_type *copy_next(boost::iterator_range<element_type *> destination) SILICIUM_OVERRIDE
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
					*i = Si::incoming_bytes(received.data(), received.data() + bytes_received);
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

		Si::source<Si::received_from_socket> &receiving()
		{
			return receiving_;
		}

		void send(boost::iterator_range<char const *> data)
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

	struct coroutine_web_server
	{
		explicit coroutine_web_server(boost::asio::io_service &io, boost::uint16_t port)
			: acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(), port))
			, clients(acceptor)
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
					auto context = Si::visit<events>(*result,
						[visitor_count](std::shared_ptr<boost::asio::ip::tcp::socket> client) -> events
					{
						auto visitor_number_ = visitor_count;
						auto client_handler = Si::erase_shared(Si::make_coroutine_generator<Si::nothing>([client, visitor_number_](Si::push_context<Si::nothing> &yield) -> void
						{
							coroutine_socket coro_socket(*client, yield);
							return serve_client(coro_socket, visitor_number_);
						}));
						return client_handler;
					},
						[](boost::system::error_code) -> events
					{
						throw std::logic_error("not implemented");
					});
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
		Si::tcp_acceptor clients;
		Si::total_consumer<Si::unique_observable<Si::nothing>> all_work;
	};

	struct thread_web_server
	{
		explicit thread_web_server(boost::asio::io_service &io, boost::uint16_t port)
			: acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(), port))
			, clients(acceptor)
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
					Si::visit<void>(*result,
						[visitor_count](std::shared_ptr<boost::asio::ip::tcp::socket> client)
					{
						auto visitor_number_ = visitor_count;
						std::thread([client, visitor_number_]
						{
							thread_socket threaded_socket(*client);
							serve_client(threaded_socket, visitor_number_);
						}).detach();
					},
						[](boost::system::error_code)
					{
						throw std::logic_error("not implemented");
					});
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
		Si::tcp_acceptor clients;
		Si::total_consumer<Si::unique_observable<Si::nothing>> all_work;
	};
}

int main()
{
	boost::asio::io_service io;

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
	io.run();
}
