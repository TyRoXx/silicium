#ifndef SILICIUM_ASIO_WRITING_OBSERVABLE_HPP
#define SILICIUM_ASIO_WRITING_OBSERVABLE_HPP

#include <silicium/error_or.hpp>

namespace Si
{
	template <class AsyncStream>
	struct writing_observable
	{
		typedef boost::system::error_code element_type;

		explicit writing_observable(AsyncStream &stream, boost::iterator_range<char const *> buffer)
			: stream(&stream)
		{
			assert(buffer.size() >= 1);
		}

		void async_get_one(observer<element_type> &receiver)
		{
			boost::asio::async_write(
				*stream,
				boost::asio::buffer(buffer.begin(), buffer.size()),
				[this, &receiver](boost::system::error_code ec, std::size_t bytes_received)
			{
				assert(ec || (bytes_received == buffer.size()));
				receiver.got_element(ec);
			});
		}

	private:

		AsyncStream *stream;
		boost::iterator_range<char const *> buffer;
	};

	template <class AsyncStream>
	auto make_writing_observable(AsyncStream &stream, boost::iterator_range<char const *> buffer)
	{
		return writing_observable<AsyncStream>(stream, buffer);
	}
}

#endif
