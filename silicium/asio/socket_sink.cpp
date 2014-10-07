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

	boost::system::error_code socket_sink::flush_append_space()
	{
		return {};
	}

	boost::system::error_code socket_sink::append(boost::iterator_range<char const *> data)
	{
		assert(m_socket);
		assert(m_yield);
		boost::system::error_code ec;
		boost::asio::async_write(*m_socket, boost::asio::buffer(data.begin(), data.size()), (*m_yield)[ec]);
		return ec;
	}
}
#endif
