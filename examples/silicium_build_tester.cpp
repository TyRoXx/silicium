#include <silicium/async_process.hpp>
#include <silicium/http/parse_request.hpp>
#include <silicium/http/generate_response.hpp>
#include <silicium/asio/tcp_acceptor.hpp>
#include <silicium/observable/spawn_coroutine.hpp>
#include <iostream>

namespace
{

}

int main()
{
	boost::asio::io_service io;
	Si::spawn_coroutine([&io](Si::spawn_context yield)
	{
		auto acceptor = Si::asio::make_tcp_acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(), 8080));
		for (;;)
		{
			auto client = (*yield.get_one(Si::ref(acceptor)))->get();
			assert(client);
		}
	});
	io.run();
}
