#include <silicium/http/receive_request.hpp>
#include <silicium/http/generate_response.hpp>
#include <silicium/asio/writing_observable.hpp>
#include <silicium/asio/tcp_acceptor.hpp>
#include <silicium/observable/transform.hpp>
#include <silicium/observable/spawn_coroutine.hpp>
#include <silicium/observable/spawn_observable.hpp>
#include <silicium/sink/iterator_sink.hpp>

namespace
{
	template <class YieldContext>
	void serve_client(boost::asio::ip::tcp::socket &client, YieldContext &&yield)
	{
		auto request = Si::http::receive_request(client, yield);
		if (request.is_error())
		{
			//The header was incomplete, maybe the connecting was closed.
			//If we want to know the reason, the error_extracting_source remembered it:
			boost::system::error_code error = request.error();
			boost::ignore_unused_variable_warning(error);
			return;
		}

		if (!request.get())
		{
			//syntax error in the request
			return;
		}

		std::vector<char> response;
		{
			auto response_writer = Si::make_container_sink(response);
			Si::http::generate_status_line(response_writer, "HTTP/1.0", "200", "OK");
			std::string const content = "Hello";
			Si::http::generate_header(response_writer, "Content-Length", boost::lexical_cast<Si::noexcept_string>(content.size()));
			Si::http::finish_headers(response_writer);
			Si::append(response_writer, content);
		}

		//you can handle the error if you want
		boost::system::error_code error = Si::asio::write(client, Si::make_memory_range(response), yield);

		//ignore shutdown failures, they do not matter here
		client.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
	}
}

int main()
{
	boost::asio::io_service io;
	Si::spawn_observable(
		Si::transform(
			Si::asio::make_tcp_acceptor(
				boost::asio::ip::tcp::acceptor(
					io,
					boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(), 8080)
				)
			),
			[](Si::asio::tcp_acceptor_result maybe_client)
			{
				auto client = maybe_client.get();
				Si::spawn_coroutine([client](Si::spawn_context yield)
				{
					serve_client(*client, yield);
				});
				return Si::nothing();
			}
		)
	);
	io.run();
}
