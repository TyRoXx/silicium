#ifndef SILICIUM_ASIO_ACCEPTING_SOURCE_HPP
#define SILICIUM_ASIO_ACCEPTING_SOURCE_HPP

#include <silicium/source.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <memory>

#if BOOST_VERSION >= 105400
#include <boost/asio/spawn.hpp>

namespace Si
{
	struct accepting_source : Si::source<std::shared_ptr<boost::asio::ip::tcp::socket>>
	{
		typedef std::shared_ptr<boost::asio::ip::tcp::socket> element_type;

		explicit accepting_source(boost::asio::ip::tcp::acceptor &acceptor, boost::asio::yield_context &yield);
		virtual boost::iterator_range<element_type const *> map_next(std::size_t) SILICIUM_OVERRIDE;
		virtual element_type *copy_next(boost::iterator_range<element_type *> destination) SILICIUM_OVERRIDE;

	private:

		boost::asio::ip::tcp::acceptor *m_acceptor;
		boost::asio::yield_context *m_yield;
	};
}
#endif

#endif
