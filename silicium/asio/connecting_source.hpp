#ifndef SILICIUM_ASIO_CONNECTING_SOURCE_HPP
#define SILICIUM_ASIO_CONNECTING_SOURCE_HPP

#include <silicium/source/source.hpp>

#if BOOST_VERSION < 105300
// for Asio
#include <algorithm>
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/variant.hpp>
#include <memory>

#if BOOST_VERSION >= 105400 && SILICIUM_HAS_EXCEPTIONS
#include <boost/asio/spawn.hpp>

namespace Si
{
	namespace asio
	{
		typedef std::shared_ptr<boost::asio::ip::tcp::socket> connecting_source_socket_ptr;
		typedef boost::variant<connecting_source_socket_ptr, boost::system::error_code> connecting_source_element_type;

		struct connecting_source
		{
			typedef connecting_source_element_type element_type;

			explicit connecting_source(boost::asio::io_service &io, boost::asio::yield_context &yield,
			                           boost::asio::ip::tcp::endpoint remote_endpoint);
			iterator_range<element_type const *> map_next(std::size_t);
			element_type *copy_next(iterator_range<element_type *> destination);

		private:
			boost::asio::io_service *io;
			boost::asio::yield_context *yield;
			boost::asio::ip::tcp::endpoint remote_endpoint;
		};

		inline connecting_source::connecting_source(boost::asio::io_service &io, boost::asio::yield_context &yield,
		                                            boost::asio::ip::tcp::endpoint remote_endpoint)
		    : io(&io)
		    , yield(&yield)
		    , remote_endpoint(remote_endpoint)
		{
		}

		inline iterator_range<connecting_source::element_type const *> connecting_source::map_next(std::size_t)
		{
			assert(io);
			assert(yield);
			return {};
		}

		inline connecting_source::element_type *connecting_source::copy_next(iterator_range<element_type *> destination)
		{
			assert(io);
			assert(yield);
			auto copied = begin(destination);
			for (; copied != end(destination); ++copied)
			{
				assert(io);
				auto socket = std::make_shared<boost::asio::ip::tcp::socket>(*io);
				boost::system::error_code error;
				assert(yield);
				socket->async_connect(remote_endpoint, (*yield)[error]);
				if (error)
				{
					*copied = error;
				}
				else
				{
					*copied = socket;
				}
			}
			return copied;
		}
	}
}
#endif

#endif
