#include <silicium/terminate_on_exception.hpp>
#include <silicium/config.hpp>
#include <iostream>
#include <boost/version.hpp>
#if BOOST_VERSION >= 105400 && SILICIUM_HAS_EXCEPTIONS

#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

int main()
{
	typedef websocketpp::server<websocketpp::config::asio> server_type;

	server_type server;

	//websocketpp thinks it is OK to spam on stderr by default. I don't think so.
	server.clear_access_channels(websocketpp::log::alevel::all);
	server.clear_error_channels(websocketpp::log::alevel::all);

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
		strong_connection->set_body("<p>Hello, world!</p><p>" + strong_connection->get_request().get_header("User-Agent") + "</p>");
		strong_connection->append_header("Content-Type", "text/html");
	});

	//a handler for websocket messages received on existing connections
	server.set_message_handler([](websocketpp::connection_hdl connection, server_type::message_ptr message)
	{

	});

	server.listen(8080);
	server.start_accept();
	server.run();
}
#else
int main()
{
	std::cerr << "websocketpp requires a recent version of Boost and exception support\n";
}
#endif
