#include <silicium/http/receive_request.hpp>
#include <silicium/http/generate_response.hpp>
#include <silicium/html/generator.hpp>
#include <silicium/asio/writing_observable.hpp>
#include <silicium/asio/tcp_acceptor.hpp>
#include <silicium/observable/transform.hpp>
#include <silicium/observable/spawn_coroutine.hpp>
#include <silicium/observable/spawn_observable.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <silicium/sink/ptr_sink.hpp>
#include <silicium/terminate_on_exception.hpp>
#include <iostream>

#define SILICIUM_EXAMPLE_AVAILABLE                                             \
	(SILICIUM_HAS_SPAWN_COROUTINE && SILICIUM_HAS_TRANSFORM_OBSERVABLE)

#if SILICIUM_EXAMPLE_AVAILABLE
namespace
{
	template <class YieldContext>
	void serve_client(boost::asio::ip::tcp::socket &client,
	                  YieldContext &&yield)
	{
		auto request = Si::http::receive_request(client, yield);
		if (request.is_error())
		{
			// The header was incomplete, maybe the connecting was closed.
			// If we want to know the reason, the error_extracting_source
			// remembered it:
			boost::system::error_code error = request.error();
			boost::ignore_unused_variable_warning(error);
			return;
		}

		if (!request.get())
		{
			// syntax error in the request
			return;
		}

		std::vector<char> response;
		{
			auto response_writer = Si::make_container_sink(response);
			Si::http::generate_status_line(
			    response_writer, "HTTP/1.0", "200", "OK");
			std::string content;
			{
				auto content_writer = Si::make_container_sink(content);
				auto html =
				    Si::html::make_generator(Si::ref_sink(content_writer));
				html.element("p", [&]
				             {
					             html.write("Hello, visitor!");
					         });
				html.element_with_text("h1", "Your request headers were:");
				Si::html::open_attributed_element(content_writer, "table");
				Si::html::add_attribute(content_writer, "border", "3px");
				Si::html::finish_attributes(content_writer);
				for (auto const &request_header : request.get()->arguments)
				{
					html.element(
					    "tr", [&]
					    {
						    html.element("td", [&]
						                 {
							                 html.write(request_header.first);
							             });
						    html.element("td", [&]
						                 {
							                 html.write(request_header.second);
							             });
						});
				}
				Si::html::close_element(content_writer, "table");
			}
			Si::http::generate_header(
			    response_writer, "Content-Length",
			    boost::lexical_cast<Si::noexcept_string>(content.size()));
			Si::http::finish_headers(response_writer);
			Si::append(response_writer, content);
		}

		// you can handle the error if you want
		boost::system::error_code error =
		    Si::asio::write(client, Si::make_memory_range(response), yield);

		// ignore shutdown failures, they do not matter here
		client.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
	}
}
#endif

int main()
{
	boost::asio::io_service io;
#if SILICIUM_EXAMPLE_AVAILABLE
	Si::spawn_observable(
	    Si::transform(Si::asio::make_tcp_acceptor(
	                      // use a unique_ptr to support older versions of Boost
	                      // where acceptor
	                      // was not movable
	                      Si::make_unique<boost::asio::ip::tcp::acceptor>(
	                          io, boost::asio::ip::tcp::endpoint(
	                                  boost::asio::ip::address_v4(), 8080))),
	                  [](Si::asio::tcp_acceptor_result maybe_client)
	                  {
		                  auto client = maybe_client.get();
		                  Si::spawn_coroutine([client](Si::spawn_context yield)
		                                      {
			                                      serve_client(*client, yield);
			                                  });
		                  return Si::unit();
		              }));
#else
	std::cerr << "This example requires coroutine support\n";
#endif
	io.run();
}
