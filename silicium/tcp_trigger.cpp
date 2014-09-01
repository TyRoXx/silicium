#include <silicium/tcp_trigger.hpp>

namespace Si
{
	tcp_trigger::tcp_trigger(boost::asio::io_service &io, boost::asio::ip::tcp::endpoint address)
		: m_acceptor(io, address, true)
	{
	}
}
