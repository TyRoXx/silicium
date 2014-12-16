#ifndef SILICIUM_ASIO_READING_OBSERVABLE_HPP
#define SILICIUM_ASIO_READING_OBSERVABLE_HPP

#include <silicium/config.hpp>
#include <silicium/error_or.hpp>
#include <silicium/memory_range.hpp>
#include <silicium/observable/observer.hpp>
#include <boost/asio/buffer.hpp>

namespace Si
{
	namespace asio
	{
		template <class AsyncStream>
		struct reading_observable
		{
			typedef error_or<memory_range> element_type;

			reading_observable() BOOST_NOEXCEPT
				: stream(nullptr)
			{
			}

			explicit reading_observable(AsyncStream &stream, mutable_memory_range buffer) BOOST_NOEXCEPT
				: stream(&stream)
				, buffer(buffer)
			{
				assert(this->buffer.size() >= 1);
			}

			template <class Observer>
			void async_get_one(Observer &&receiver)
			{
				stream->async_read_some(
					boost::asio::buffer(buffer.begin(), buffer.size()),
					[this, receiver = std::forward<Observer>(receiver)](boost::system::error_code ec, std::size_t bytes_received) mutable
				{
					if (ec)
					{
						std::forward<Observer>(receiver).got_element(ec);
					}
					else
					{
						assert(bytes_received <= static_cast<std::size_t>(this->buffer.size()));
						std::forward<Observer>(receiver).got_element(make_memory_range(this->buffer.begin(), this->buffer.begin() + bytes_received));
					}
				});
			}

		private:

			AsyncStream *stream;
			mutable_memory_range buffer;
		};

		template <class AsyncStream>
		auto make_reading_observable(AsyncStream &stream, mutable_memory_range buffer)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			-> reading_observable<AsyncStream>
#endif
		{
			return reading_observable<AsyncStream>(stream, buffer);
		}
	}
}

#endif
