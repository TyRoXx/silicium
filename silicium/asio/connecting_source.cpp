#include <silicium/asio/connecting_source.hpp>

#if BOOST_VERSION >= 105400
namespace Si
{
	connecting_source::connecting_source(boost::asio::io_service &io, boost::asio::yield_context &yield, boost::asio::ip::tcp::endpoint remote_endpoint)
		: io(&io)
		, yield(&yield)
		, remote_endpoint(remote_endpoint)
	{
	}

	boost::iterator_range<connecting_source::element_type const *> connecting_source::map_next(std::size_t)
	{
		assert(io);
		assert(yield);
		return {};
	}

	connecting_source::element_type *connecting_source::copy_next(boost::iterator_range<element_type *> destination)
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
#endif
