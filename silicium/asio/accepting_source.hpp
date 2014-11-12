#ifndef SILICIUM_ASIO_ACCEPTING_SOURCE_HPP
#define SILICIUM_ASIO_ACCEPTING_SOURCE_HPP

#include <silicium/source/source.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <memory>

#if BOOST_VERSION >= 105400
#include <boost/asio/spawn.hpp>

namespace Si
{
	namespace asio
	{
		struct accepting_source : Si::source<std::shared_ptr<boost::asio::ip::tcp::socket>>
		{
			typedef std::shared_ptr<boost::asio::ip::tcp::socket> element_type;

			explicit accepting_source(boost::asio::ip::tcp::acceptor &acceptor, boost::asio::yield_context &yield);
			virtual iterator_range<element_type const *> map_next(std::size_t) SILICIUM_OVERRIDE;
			virtual element_type *copy_next(iterator_range<element_type *> destination) SILICIUM_OVERRIDE;

		private:

			boost::asio::ip::tcp::acceptor *m_acceptor;
			boost::asio::yield_context *m_yield;
		};

		inline accepting_source::accepting_source(boost::asio::ip::tcp::acceptor &acceptor, boost::asio::yield_context &yield)
			: m_acceptor(&acceptor)
			, m_yield(&yield)
		{
		}

		inline iterator_range<accepting_source::element_type const *> accepting_source::map_next(std::size_t)
		{
			return iterator_range<element_type const *>();
		}

		inline accepting_source::element_type *accepting_source::copy_next(iterator_range<element_type *> destination)
		{
			assert(m_acceptor);
			assert(m_yield);
			for (auto &client : destination)
			{
				assert(m_acceptor);
				client = std::make_shared<boost::asio::ip::tcp::socket>(m_acceptor->get_io_service());
				assert(m_yield);
				m_acceptor->async_accept(*client, *m_yield);
			}
			return destination.end();
		}
	}
}
#endif

#endif
