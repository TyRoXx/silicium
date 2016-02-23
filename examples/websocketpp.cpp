#include <silicium/terminate_on_exception.hpp>
#include <silicium/config.hpp>
#include <silicium/html/generator.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <iostream>
#include <boost/version.hpp>
#if BOOST_VERSION >= 105400 && SILICIUM_HAS_EXCEPTIONS

#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

#define LOG(...)                                                               \
	do                                                                         \
	{                                                                          \
		std::cerr << __VA_ARGS__ << '\n';                                      \
	} while (false)

namespace
{
	template <class CharSink>
	void generate_html_landing_page(
	    CharSink &&page, websocketpp::http::parser::request const &request)
	{
		auto doc = Si::html::make_generator(page);
		doc.raw("<!doctype html>\n");
		doc("html", [&]
		    {
			    doc("head", [&]
			        {
				        doc("meta",
				            [&]
				            {
					            doc.attribute("charset", "utf-8");
					        },
				            Si::html::empty);
				        doc("title", [&]
				            {
					            doc.write("Silicium websocketpp experiment");
					        });
				        doc.raw(
				            R"QQQ(<link rel="stylesheet" href="//cdnjs.cloudflare.com/ajax/libs/highlight.js/8.6/styles/github.min.css">)QQQ"
				            R"QQQ(<script src="//cdnjs.cloudflare.com/ajax/libs/highlight.js/8.6/highlight.min.js"></script>)QQQ"
				            R"QQQ(<link rel="stylesheet" href="http://yui.yahooapis.com/pure/0.6.0/pure-min.css">)QQQ");
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
				        doc("pre", [&]
				            {
					            doc("code",
					                [&]
					                {
						                doc.attribute("class", "javascript");
						            },
					                [&]
					                {
						                doc.raw(
#include "websocketpp.hpp.js"
						                    );
						            });
					        });
				        doc.raw(
				            R"QQQ(
<form class="pure-form" action="#" method="post">
	<fieldset class="pure-group">
		<input type="text" class="pure-input-1-2" name="username" placeholder="username">
		<input type="text" class="pure-input-1-2" name="password" placeholder="password">
	</fieldset>
	<button type="submit" class="pure-button pure-input-1-2 pure-button-primary">Log in</button>
</form>
)QQQ");
				    });
			});
	}

	void run_example_webserver()
	{
		typedef websocketpp::server<websocketpp::config::asio> server_type;

		server_type server;

		// websocketpp thinks it is OK to spam on stderr by default. I don't
		// think so.
		server.clear_access_channels(websocketpp::log::alevel::all);
		server.clear_error_channels(websocketpp::log::alevel::all);

		server.set_reuse_addr(true);
		server.init_asio();

		// a handler for any non-websocket HTTP requests
		server.set_http_handler(
		    [&server](websocketpp::connection_hdl weak_connection)
		    {
			    server_type::connection_ptr const strong_connection =
			        server.get_con_from_hdl(weak_connection);
			    assert(strong_connection);

			    strong_connection->append_header("Connection", "close");
			    strong_connection->append_header(
			        "Content-Type", "text/html; charset=utf-8");

			    websocketpp::http::parser::request const &request =
			        strong_connection->get_request();

			    if (request.get_method() == "GET")
			    {
				    strong_connection->set_status(
				        websocketpp::http::status_code::ok);

				    std::string body;
				    auto body_writer = Si::make_container_sink(body);
				    generate_html_landing_page(body_writer, request);
				    strong_connection->set_body(std::move(body));
			    }

			    else if (request.get_method() == "POST")
			    {
				    websocketpp::http::parameter_list parameters;
				    request.parse_parameter_list(
				        request.get_body(), parameters);

				    strong_connection->set_status(
				        websocketpp::http::status_code::ok);

				    std::string body = "logged in";
				    strong_connection->set_body(std::move(body));
			    }

			    else
			    {
				    strong_connection->set_status(
				        websocketpp::http::status_code::method_not_allowed);
			    }
			});

		server.set_open_handler(
		    [&server](websocketpp::connection_hdl weak_connection)
		    {
			    server_type::connection_ptr const strong_connection =
			        server.get_con_from_hdl(weak_connection);
			    assert(strong_connection);
			    LOG("Incoming connected: " << strong_connection->get_host());

			    strong_connection->send("Hello client");
			});

		server.set_close_handler(
		    [&server](websocketpp::connection_hdl weak_connection)
		    {
			    server_type::connection_ptr const strong_connection =
			        server.get_con_from_hdl(weak_connection);
			    assert(strong_connection);
			    LOG("Websocket disconnected: "
			        << strong_connection->get_host());
			});

		// a handler for websocket messages received on existing connections
		server.set_message_handler(
		    [&server](websocketpp::connection_hdl weak_connection,
		              server_type::message_ptr message)
		    {
			    server_type::connection_ptr const strong_connection =
			        server.get_con_from_hdl(weak_connection);
			    assert(strong_connection);
			    LOG("Websocket " << strong_connection->get_host()
			                     << " sent: " << message->get_payload());
			});

		server.listen(8080);
		server.start_accept();
		do
		{
			server.run();
		} while (!server.stopped());
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
		std::cerr <<
#if SILICIUM_HAS_RTTI
		    typeid(ex).name() << ": " <<
#endif
		    ex.what() << '\n';
		return 1;
	}
}
#else
int main()
{
	std::cerr << "websocketpp requires a recent version of Boost and exception "
	             "support\n";
}
#endif
