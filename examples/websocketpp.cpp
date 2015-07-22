#include <silicium/terminate_on_exception.hpp>
#include <silicium/config.hpp>
#include <silicium/html.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <iostream>
#include <boost/version.hpp>
#if BOOST_VERSION >= 105400 && SILICIUM_HAS_EXCEPTIONS

#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

#define LOG(...) do { std::cerr << __VA_ARGS__ << '\n'; } while (false)

namespace
{
	template <class CharSink>
	void generate_html_landing_page(CharSink &&page, websocketpp::http::parser::request const &request)
	{
		auto doc = Si::html::make_generator(page);
		doc.raw("<!doctype html>\n");
		doc("html", [&]
		{
			doc("head", [&]
			{
				doc("meta", [&]
				{
					doc.attribute("charset", "utf-8");
				},
					[&]
				{
				});
				doc("title", [&]
				{
					doc.write("Silicium websocketpp experiment");
				});
				doc("script", [&]
				{
					doc.raw(
#include "websocketpp.hpp.js"
					);
				});
			});
			doc("body", [&]
			{
				doc("p", [&]
				{
					doc.write("Hello, world!");
				});
				doc.write(request.get_header("User-Agent"));
			});
		});
	}

	void run_example_webserver()
	{
		typedef websocketpp::server<websocketpp::config::asio> server_type;

		server_type server;

		//websocketpp thinks it is OK to spam on stderr by default. I don't think so.
		server.clear_access_channels(websocketpp::log::alevel::all);
		server.clear_error_channels(websocketpp::log::alevel::all);

		server.set_reuse_addr(true);
		server.init_asio();

		//a handler for any non-websocket HTTP requests
		server.set_http_handler([&server](websocketpp::connection_hdl weak_connection)
		{
			server_type::connection_ptr const strong_connection = server.get_con_from_hdl(weak_connection);
			assert(strong_connection);

			if (strong_connection->get_request().get_method() != "GET")
			{
				strong_connection->set_status(websocketpp::http::status_code::method_not_allowed);
				return;
			}

			strong_connection->set_status(websocketpp::http::status_code::ok);

			{
				std::string body;
				auto body_writer = Si::make_container_sink(body);
				generate_html_landing_page(body_writer, strong_connection->get_request());
				strong_connection->set_body(std::move(body));
			}
			strong_connection->append_header("Connection", "close");
			strong_connection->append_header("Content-Type", "text/html; charset=utf-8");
		});

		server.set_open_handler([&server](websocketpp::connection_hdl weak_connection)
		{
			server_type::connection_ptr const strong_connection = server.get_con_from_hdl(weak_connection);
			assert(strong_connection);
			LOG("Incoming connected: " << strong_connection->get_host());

			strong_connection->send("Hello client");
		});

		server.set_close_handler([&server](websocketpp::connection_hdl weak_connection)
		{
			server_type::connection_ptr const strong_connection = server.get_con_from_hdl(weak_connection);
			assert(strong_connection);
			LOG("Websocket disconnected: " << strong_connection->get_host());
		});

		//a handler for websocket messages received on existing connections
		server.set_message_handler([&server](websocketpp::connection_hdl weak_connection, server_type::message_ptr message)
		{
			server_type::connection_ptr const strong_connection = server.get_con_from_hdl(weak_connection);
			assert(strong_connection);
			LOG("Websocket " << strong_connection->get_host() << " sent: " << message->get_payload());
		});

		server.listen(8080);
		server.start_accept();
		do
		{
			server.run();
		}
		while (!server.stopped());
	}
}

int main()
{
	try
	{
		run_example_webserver();
	}
	catch (std::exception const &ex)
	{
		std::cerr << typeid(ex).name() << ": " << ex.what() << '\n';
		return 1;
	}
}
#else
int main()
{
	std::cerr << "websocketpp requires a recent version of Boost and exception support\n";
}
#endif
