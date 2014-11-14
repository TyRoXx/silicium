#ifndef SILICIUM_ASIO_WRITING_OBSERVABLE_HPP
#define SILICIUM_ASIO_WRITING_OBSERVABLE_HPP

#include <silicium/config.hpp>
#include <silicium/memory_range.hpp>
#include <silicium/override.hpp>
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
		template <class AsyncStream, class BufferObservable>
		struct writing_observable : private observer<memory_range>
		{
			typedef boost::system::error_code element_type;

			writing_observable()
				: stream(nullptr)
				, receiver_(nullptr)
			{
			}

			explicit writing_observable(AsyncStream &stream, BufferObservable buffers)
				: stream(&stream)
				, buffers(std::move(buffers))
				, receiver_(nullptr)
			{
			}

			void async_get_one(observer<element_type> &receiver)
			{
				assert(!receiver_);
				assert(stream);
				receiver_ = &receiver;
				buffers.async_get_one(static_cast<observer<memory_range> &>(*this));
			}

		private:

			AsyncStream *stream;
			BufferObservable buffers;
			observer<element_type> *receiver_;

			virtual void got_element(memory_range value) SILICIUM_OVERRIDE
			{
				assert(stream);
				boost::asio::async_write(
					*stream,
					boost::asio::buffer(value.begin(), value.size()),
					[this](boost::system::error_code ec, std::size_t bytes_sent)
				{
					(void)bytes_sent;
					Si::exchange(receiver_, nullptr)->got_element(ec);
				});
			}

			virtual void ended() SILICIUM_OVERRIDE
			{
				Si::exchange(receiver_, nullptr)->ended();
			}
		};

		template <class AsyncStream, class BufferObservable>
		auto make_writing_observable(AsyncStream &stream, BufferObservable &&buffers)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			-> writing_observable<AsyncStream, typename std::decay<BufferObservable>::type>
#endif
		{
			return writing_observable<AsyncStream, typename std::decay<BufferObservable>::type>(stream, std::forward<BufferObservable>(buffers));
		}

		template <class AsyncStream>
		boost::system::error_code write(AsyncStream &stream, memory_range data, yield_context yield)
		{
			auto writer = make_writing_observable(stream, make_constant_observable(data));
			return *yield.get_one(writer);
		}
	}
}

#endif
