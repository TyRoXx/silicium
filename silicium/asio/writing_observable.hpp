#ifndef SILICIUM_ASIO_WRITING_OBSERVABLE_HPP
#define SILICIUM_ASIO_WRITING_OBSERVABLE_HPP

#include <silicium/config.hpp>
#include <silicium/memory_range.hpp>
#include <silicium/exchange.hpp>
#include <silicium/observable/yield_context.hpp>
#include <silicium/observable/constant.hpp>
#include <silicium/observable/observer.hpp>
#include <silicium/iterator_range.hpp>
#include <boost/asio/write.hpp>

namespace Si
{
	namespace asio
	{
		template <class AsyncStream>
		struct writing_observable
		{
			typedef boost::system::error_code element_type;

			writing_observable()
				: stream(nullptr)
			{
			}

			explicit writing_observable(AsyncStream &stream)
				: stream(&stream)
			{
			}

			void set_buffer(memory_range buffer)
			{
				m_buffer = buffer;
			}

			template <class Observer>
			void async_get_one(Observer &&receiver)
			{
				assert(stream);
				boost::asio::async_write(
					*stream,
					boost::asio::buffer(m_buffer.begin(), m_buffer.size()),
					[receiver = std::forward<Observer>(receiver)](boost::system::error_code ec, std::size_t bytes_sent) mutable
				{
					(void)bytes_sent;
					std::forward<Observer>(receiver).got_element(ec);
				});
			}

		private:

			AsyncStream *stream;
			memory_range m_buffer;
		};

		template <class AsyncStream>
		auto make_writing_observable(AsyncStream &stream)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			-> writing_observable<AsyncStream>
#endif
		{
			return writing_observable<AsyncStream>(stream);
		}

		template <class AsyncStream>
		boost::system::error_code write(AsyncStream &stream, memory_range data, yield_context yield)
		{
			auto writer = make_writing_observable(stream);
			writer.set_buffer(data);
			return *yield.get_one(writer);
		}
	}
}

#endif
