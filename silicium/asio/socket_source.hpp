#ifndef SILICIUM_ASIO_SOCKET_SOURCE_HPP
#define SILICIUM_ASIO_SOCKET_SOURCE_HPP

#include <silicium/source.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>

namespace Si
{
	struct socket_source : Si::source<char>
	{
		explicit socket_source(boost::asio::ip::tcp::socket &socket, boost::asio::yield_context &yield);
		virtual boost::iterator_range<char const *> map_next(std::size_t size) SILICIUM_OVERRIDE;
		virtual char *copy_next(boost::iterator_range<char *> destination) SILICIUM_OVERRIDE;
		virtual boost::uintmax_t minimum_size() SILICIUM_OVERRIDE;
		virtual boost::optional<boost::uintmax_t> maximum_size() SILICIUM_OVERRIDE;

	private:

		boost::asio::ip::tcp::socket &m_socket;
		boost::asio::yield_context &m_yield;
	};
}

#endif