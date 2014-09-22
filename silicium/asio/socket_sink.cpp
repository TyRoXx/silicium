#include <silicium/asio/socket_sink.hpp>

#if BOOST_VERSION >= 105400
#include <boost/asio/write.hpp>

namespace Si
{
	socket_sink::socket_sink(boost::asio::ip::tcp::socket &socket, boost::asio::yield_context &yield)
		: m_socket(&socket)
		, m_yield(&yield)
	{
	}

	boost::iterator_range<char *> socket_sink::make_append_space(std::size_t)
	{
		return boost::iterator_range<char *>();
	}

	void socket_sink::flush_append_space()
	{
	}

	void socket_sink::append(boost::iterator_range<char const *> data)
	{
		assert(m_socket);
		assert(m_yield);
		boost::asio::async_write(*m_socket, boost::asio::buffer(data.begin(), data.size()), *m_yield);
	}
}
#endif
