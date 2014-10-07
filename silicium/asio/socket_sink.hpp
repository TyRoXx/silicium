#ifndef SILICIUM_ASIO_SOCKET_SINK_HPP
#define SILICIUM_ASIO_SOCKET_SINK_HPP

#include <silicium/sink.hpp>
#include <boost/asio/ip/tcp.hpp>

#if BOOST_VERSION >= 105400
#include <boost/asio/spawn.hpp>

namespace Si
{
	struct socket_sink : Si::sink<char, boost::system::error_code>
	{
		explicit socket_sink(boost::asio::ip::tcp::socket &socket, boost::asio::yield_context &yield);
		virtual boost::iterator_range<char *> make_append_space(std::size_t size) SILICIUM_OVERRIDE;
		virtual boost::system::error_code flush_append_space() SILICIUM_OVERRIDE;
		virtual boost::system::error_code append(boost::iterator_range<char const *> data) SILICIUM_OVERRIDE;

	private:

		boost::asio::ip::tcp::socket *m_socket;
		boost::asio::yield_context *m_yield;
	};
}
#endif

#endif
