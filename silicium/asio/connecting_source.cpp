#include <silicium/asio/connecting_source.hpp>

namespace Si
{
	connecting_source::connecting_source(boost::asio::io_service &io, boost::asio::yield_context &yield, boost::asio::ip::tcp::endpoint remote_endpoint)
		: io(io)
		, yield(yield)
		, remote_endpoint(remote_endpoint)
	{
	}

	boost::iterator_range<connecting_source::element_type const *> connecting_source::map_next(std::size_t)
	{
		return {};
	}

	connecting_source::element_type *connecting_source::copy_next(boost::iterator_range<element_type *> destination)
	{
		auto copied = begin(destination);
		for (; copied != end(destination); ++copied)
		{
			auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io);
			boost::system::error_code error;
			socket->async_connect(remote_endpoint, yield[error]);
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

	boost::uintmax_t connecting_source::minimum_size()
	{
		return 0;
	}

	boost::optional<boost::uintmax_t> connecting_source::maximum_size()
	{
		return boost::none;
	}

	std::size_t connecting_source::skip(std::size_t count)
	{
		std::size_t skipped = 0;
		for (; (skipped < count) && Si::get(*this); ++skipped)
		{
		}
		return skipped;
	}
}
