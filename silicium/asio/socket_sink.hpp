#ifndef SILICIUM_ASIO_SOCKET_SINK_HPP
#define SILICIUM_ASIO_SOCKET_SINK_HPP

#include <silicium/sink/sink.hpp>
#include <algorithm>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>

#define SILICIUM_HAS_ASIO_SOCKET_SINK                                          \
    (!SILICIUM_AVOID_BOOST_COROUTINE && (BOOST_VERSION >= 105400) &&           \
     SILICIUM_HAS_EXCEPTIONS)

#if SILICIUM_HAS_ASIO_SOCKET_SINK
#include <boost/asio/spawn.hpp>

namespace Si
{
    namespace asio
    {
        struct socket_sink
        {
            typedef char element_type;
            typedef boost::system::error_code error_type;

            explicit socket_sink(boost::asio::ip::tcp::socket &socket,
                                 boost::asio::yield_context &yield);
            boost::system::error_code append(iterator_range<char const *> data);

        private:
            boost::asio::ip::tcp::socket *m_socket;
            boost::asio::yield_context *m_yield;
        };

        inline socket_sink::socket_sink(boost::asio::ip::tcp::socket &socket,
                                        boost::asio::yield_context &yield)
            : m_socket(&socket)
            , m_yield(&yield)
        {
        }

        inline boost::system::error_code
        socket_sink::append(iterator_range<char const *> data)
        {
            assert(m_socket);
            assert(m_yield);
            boost::system::error_code ec;
            boost::asio::async_write(
                *m_socket,
                boost::asio::buffer(
                    data.begin(), static_cast<std::size_t>(data.size())),
                (*m_yield)[ec]);
            return ec;
        }
    }
}
#endif

#endif
