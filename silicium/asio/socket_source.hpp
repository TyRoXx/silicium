#ifndef SILICIUM_ASIO_SOCKET_SOURCE_HPP
#define SILICIUM_ASIO_SOCKET_SOURCE_HPP

#include <silicium/source/source.hpp>
#include <boost/asio/ip/tcp.hpp>

#if BOOST_VERSION >= 105400
#include <boost/asio/spawn.hpp>

namespace Si
{
	namespace asio
	{
		struct socket_source : Si::source<char>
		{
			explicit socket_source(boost::asio::ip::tcp::socket &socket, boost::asio::yield_context &yield);
			virtual iterator_range<char const *> map_next(std::size_t size) SILICIUM_OVERRIDE;
			virtual char *copy_next(iterator_range<char *> destination) SILICIUM_OVERRIDE;

		private:

			boost::asio::ip::tcp::socket *m_socket;
			boost::asio::yield_context *m_yield;
		};

		inline socket_source::socket_source(boost::asio::ip::tcp::socket &socket, boost::asio::yield_context &yield)
			: m_socket(&socket)
			, m_yield(&yield)
		{
		}

		inline iterator_range<char const *> socket_source::map_next(std::size_t)
		{
			return iterator_range<char const *>();
		}

		inline char *socket_source::copy_next(iterator_range<char *> destination)
		{
			assert(m_socket);
			assert(m_yield);
			size_t const read = m_socket->async_read_some(boost::asio::buffer(destination.begin(), destination.size()), *m_yield);
			return destination.begin() + read;
		}
	}
}
#endif

#endif
