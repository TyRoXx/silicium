#ifndef SILICIUM_ASIO_SOCKET_SOURCE_HPP
#define SILICIUM_ASIO_SOCKET_SOURCE_HPP

#include <silicium/source/source.hpp>
#include <algorithm>
#include <boost/asio/ip/tcp.hpp>

#if BOOST_VERSION >= 105400
#include <boost/asio/spawn.hpp>

namespace Si
{
	namespace asio
	{
		template <class YieldContext>
		struct basic_socket_source : Si::source<char>
		{
			explicit basic_socket_source(boost::asio::ip::tcp::socket &socket, YieldContext &yield);
			virtual iterator_range<char const *> map_next(std::size_t size) SILICIUM_OVERRIDE;
			virtual char *copy_next(iterator_range<char *> destination) SILICIUM_OVERRIDE;

		private:

			boost::asio::ip::tcp::socket *m_socket;
			YieldContext *m_yield;
		};

		template <class YieldContext>
		basic_socket_source<YieldContext>::basic_socket_source(boost::asio::ip::tcp::socket &socket, YieldContext &yield)
			: m_socket(&socket)
			, m_yield(&yield)
		{
		}

		template <class YieldContext>
		iterator_range<char const *> basic_socket_source<YieldContext>::map_next(std::size_t)
		{
			return iterator_range<char const *>();
		}

		template <class YieldContext>
		char *basic_socket_source<YieldContext>::copy_next(iterator_range<char *> destination)
		{
			assert(m_socket);
			assert(m_yield);
			size_t const read = m_socket->async_read_some(boost::asio::buffer(destination.begin(), destination.size()), *m_yield);
			return destination.begin() + read;
		}

		typedef basic_socket_source<boost::asio::yield_context> socket_source;
	}
}
#endif

#endif
