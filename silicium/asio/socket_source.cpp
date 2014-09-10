#include <silicium/asio/socket_source.hpp>

namespace Si
{
	socket_source::socket_source(boost::asio::ip::tcp::socket &socket, boost::asio::yield_context &yield)
		: m_socket(&socket)
		, m_yield(&yield)
	{
	}

	boost::iterator_range<char const *> socket_source::map_next(std::size_t)
	{
		return boost::iterator_range<char const *>();
	}

	char *socket_source::copy_next(boost::iterator_range<char *> destination)
	{
		assert(m_socket);
		assert(m_yield);
		size_t const read = m_socket->async_read_some(boost::asio::buffer(destination.begin(), destination.size()), *m_yield);
		return destination.begin() + read;
	}
}
