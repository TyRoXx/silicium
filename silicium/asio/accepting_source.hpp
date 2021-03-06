#ifndef SILICIUM_ASIO_ACCEPTING_SOURCE_HPP
#define SILICIUM_ASIO_ACCEPTING_SOURCE_HPP

#include <algorithm>
#include <silicium/source/source.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <memory>

#define SILICIUM_HAS_ASIO_ACCEPTING_SOURCE                                     \
    (BOOST_VERSION >= 105400 && SILICIUM_HAS_EXCEPTIONS &&                     \
     !SILICIUM_AVOID_BOOST_COROUTINE)

#if SILICIUM_HAS_ASIO_ACCEPTING_SOURCE
#include <boost/asio/spawn.hpp>

namespace Si
{
    namespace asio
    {
        struct accepting_source
        {
            typedef std::shared_ptr<boost::asio::ip::tcp::socket> element_type;

            explicit accepting_source(boost::asio::ip::tcp::acceptor &acceptor,
                                      boost::asio::yield_context &yield);
            iterator_range<element_type const *> map_next(std::size_t);
            element_type *copy_next(iterator_range<element_type *> destination);

        private:
            boost::asio::ip::tcp::acceptor *m_acceptor;
            boost::asio::yield_context *m_yield;
        };

        inline accepting_source::accepting_source(
            boost::asio::ip::tcp::acceptor &acceptor,
            boost::asio::yield_context &yield)
            : m_acceptor(&acceptor)
            , m_yield(&yield)
        {
        }

        inline iterator_range<accepting_source::element_type const *>
            accepting_source::map_next(std::size_t)
        {
            return iterator_range<element_type const *>();
        }

        inline accepting_source::element_type *
        accepting_source::copy_next(iterator_range<element_type *> destination)
        {
            assert(m_acceptor);
            assert(m_yield);
            for (auto &client : destination)
            {
                assert(m_acceptor);
                client = std::make_shared<boost::asio::ip::tcp::socket>(
                    m_acceptor->get_io_service());
                assert(m_yield);
                m_acceptor->async_accept(*client, *m_yield);
            }
            return destination.end();
        }
    }
}
#endif

#endif
