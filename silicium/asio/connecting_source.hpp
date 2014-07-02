#ifndef SILICIUM_ASIO_CONNECTING_SOURCE_HPP
#define SILICIUM_ASIO_CONNECTING_SOURCE_HPP

#include <silicium/source.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/variant.hpp>
#include <memory>

namespace Si
{
	typedef std::shared_ptr<boost::asio::ip::tcp::socket> connecting_source_socket_ptr;
	typedef boost::variant<connecting_source_socket_ptr, boost::system::error_code> connecting_source_element_type;

	struct connecting_source : Si::source<connecting_source_element_type>
	{
		typedef connecting_source_element_type element_type;

		explicit connecting_source(boost::asio::io_service &io, boost::asio::yield_context &yield, boost::asio::ip::tcp::endpoint remote_endpoint);
		virtual boost::iterator_range<element_type const *> map_next(std::size_t) SILICIUM_OVERRIDE;
		virtual element_type *copy_next(boost::iterator_range<element_type *> destination) SILICIUM_OVERRIDE;
		virtual boost::uintmax_t minimum_size() SILICIUM_OVERRIDE;
		virtual boost::optional<boost::uintmax_t> maximum_size() SILICIUM_OVERRIDE;
		virtual std::size_t skip(std::size_t count) SILICIUM_OVERRIDE;

	private:

		boost::asio::io_service &io;
		boost::asio::yield_context &yield;
		boost::asio::ip::tcp::endpoint remote_endpoint;
	};
}

#endif
