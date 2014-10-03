#ifndef SILICIUM_ASIO_READING_OBSERVABLE_HPP
#define SILICIUM_ASIO_READING_OBSERVABLE_HPP

#include <silicium/config.hpp>
#include <silicium/error_or.hpp>
#include <silicium/asio/socket_observable.hpp>
#include <boost/range/iterator_range.hpp>

namespace Si
{
	template <class AsyncStream>
	struct reading_observable
	{
		typedef error_or<incoming_bytes> element_type;

		explicit reading_observable(AsyncStream &stream, boost::iterator_range<char *> buffer)
			: stream(&stream)
		{
			assert(buffer.size() >= 1);
		}

		void async_get_one(observer<element_type> &receiver)
		{
			stream->async_read_some(
				boost::asio::buffer(buffer.begin(), buffer.size()),
				[this, &receiver](boost::system::error_code ec, std::size_t bytes_received)
			{
				if (ec)
				{
					receiver.got_element(ec);
				}
				else
				{
					assert(bytes_received <= buffer.size());
					receiver.got_element(incoming_bytes(buffer.begin(), buffer.begin() + bytes_received));
				}
			});
		}

	private:

		AsyncStream *stream;
		boost::iterator_range<char *> buffer;
	};

	template <class AsyncStream>
	auto make_reading_observable(AsyncStream &stream, boost::iterator_range<char *> buffer)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> reading_observable<AsyncStream>
#endif
	{
		return reading_observable<AsyncStream>(stream, buffer);
	}
}

#endif
