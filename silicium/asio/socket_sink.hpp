#ifndef SILICIUM_ASIO_SOCKET_SINK_HPP
#define SILICIUM_ASIO_SOCKET_SINK_HPP

#include <silicium/sink.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>

namespace Si
{
	struct socket_sink : Si::sink<char>
	{
		explicit socket_sink(boost::asio::ip::tcp::socket &socket, boost::asio::yield_context &yield);
		virtual boost::iterator_range<char *> make_append_space(std::size_t size) SILICIUM_OVERRIDE;
		virtual void flush_append_space() SILICIUM_OVERRIDE;
		virtual void append(boost::iterator_range<char const *> data) SILICIUM_OVERRIDE;

	private:

		boost::asio::ip::tcp::socket *m_socket = nullptr;
		boost::asio::yield_context *m_yield = nullptr;
	};
}

#endif
