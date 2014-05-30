#ifndef SILICIUM_ASIO_ACCEPTING_SOURCE_HPP
#define SILICIUM_ASIO_ACCEPTING_SOURCE_HPP

#include <silicium/source.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <memory>

namespace Si
{

	struct accepting_source : Si::source<std::shared_ptr<boost::asio::ip::tcp::socket>>
	{
		typedef std::shared_ptr<boost::asio::ip::tcp::socket> element_type;

		explicit accepting_source(boost::asio::ip::tcp::acceptor &acceptor, boost::asio::yield_context &yield);
		virtual boost::iterator_range<element_type const *> map_next(std::size_t) SILICIUM_OVERRIDE;
		virtual element_type *copy_next(boost::iterator_range<element_type *> destination) SILICIUM_OVERRIDE;
		virtual boost::uintmax_t minimum_size() SILICIUM_OVERRIDE;
		virtual boost::optional<boost::uintmax_t> maximum_size() SILICIUM_OVERRIDE;
		virtual std::size_t skip(std::size_t count) SILICIUM_OVERRIDE;

	private:

		boost::asio::ip::tcp::acceptor &m_acceptor;
		boost::asio::yield_context &m_yield;
	};
}

#endif
