#include <silicium/http/http.hpp>
#include <silicium/asio/accepting_source.hpp>
#include <silicium/asio/socket_sink.hpp>
#include <silicium/asio/socket_source.hpp>
#include <silicium/source/virtualized_source.hpp>
#include <silicium/source/buffering_source.hpp>
#include <silicium/config.hpp>
#include <silicium/sink/ptr_sink.hpp>
#include <silicium/sink/buffering_sink.hpp>
#include <silicium/terminate_on_exception.hpp>
#include <boost/format.hpp>
#include <iostream>

#define SILICIUM_EXAMPLE_AVAILABLE (SILICIUM_HAS_ASIO_ACCEPTING_SOURCE && SILICIUM_HAS_BUFFERING_SINK)

#if SILICIUM_EXAMPLE_AVAILABLE
namespace
{
	void serve_client(
		std::shared_ptr<boost::asio::ip::tcp::socket> client,
		boost::asio::yield_context yield)
	{
		try
		{
			Si::asio::socket_source receiver(*client, yield);
			Si::asio::socket_sink sender(*client, yield);
			auto buffered_sender = Si::make_buffering_sink(Si::ref_sink(sender));
			auto buffered_receiver = Si::make_buffer(receiver, 4096);
			Si::optional<Si::http::request> const header = Si::http::parse_request(buffered_receiver);
			if (!header)
			{
				Si::http::response response;
				response.status = 400;
				response.status_text = "Bad Request";
				response.http_version = "HTTP/1.0";
				Si::http::generate_response(buffered_sender, response);
				buffered_sender.flush();
				return;
			}

			auto const client_endpoint = client->remote_endpoint();
			auto const client_name = boost::str(boost::format("%1%:%2%") % client_endpoint.address() % client_endpoint.port());
			std::string const content = "Hello, " + client_name + "!";

			Si::http::response response;
			response.arguments = Si::make_unique<std::map<Si::noexcept_string, Si::noexcept_string>>();
			response.status = 200;
			response.status_text = "OK";
			response.http_version = "HTTP/1.0";
			(*response.arguments)["Content-Length"] = boost::lexical_cast<Si::noexcept_string>(content.size());
			(*response.arguments)["Connection"] = "close";
			(*response.arguments)["Content-Type"] = "text/html";
			Si::http::generate_response(buffered_sender, response);
			Si::append(buffered_sender, content);
			buffered_sender.flush();
		}
		catch (boost::system::system_error const &)
		{
			//ignore disconnect
		}
	}
}
#endif

int main()
{
	boost::asio::io_service io;
	boost::asio::ip::tcp::acceptor acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(), 8080));
#if SILICIUM_EXAMPLE_AVAILABLE
	boost::asio::spawn(io, [&acceptor](boost::asio::yield_context yield)
	{
		auto clients = Si::virtualize_source(Si::asio::accepting_source(acceptor, yield));
		for (auto client : Si::make_buffer(clients, 1))
		{
			boost::asio::spawn(acceptor.get_io_service(), std::bind(serve_client, client, std::placeholders::_1));
		}
	});
#else
	std::cerr << "This example requires boost::asio::spawn\n";
#endif
	io.run();
}
