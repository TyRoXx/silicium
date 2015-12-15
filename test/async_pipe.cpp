#include <silicium/sink/sink.hpp>
#include <silicium/source/source.hpp>
#include <silicium/function.hpp>
#include <silicium/exchange.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/asio/io_service.hpp>

// do not want to port this to VC++ 2013
#if SILICIUM_COMPILER_GENERATES_MOVES
namespace Si
{
	template <class Message>
	SILICIUM_TRAIT_WITH_TYPEDEFS(Awaitable, typedef Message message_type;
	                             , ((async_wait, (1, (function<void(message_type)>)), void)))

	template <class Element>
	struct async_pipe_reader
	{
		typename Source<Element>::box buffer;
		typename Awaitable<nothing>::box readable;
	};

	template <class Element>
	struct async_pipe_writer
	{
		typename Sink<Element, success>::box buffer;
		typename Awaitable<nothing>::box writable;
	};

	template <class Element>
	struct async_pipe
	{
		async_pipe_reader<Element> reader;
		async_pipe_writer<Element> writer;
	};

	namespace detail
	{
		template <class Element, class Dispatcher>
		struct shared_state
		{
			boost::circular_buffer<Element> buffer;
			Dispatcher &dispatcher;
			function<void(nothing)> readable_callback;
			function<void(nothing)> writeable_callback;

			shared_state(std::size_t buffered_elements, Dispatcher &dispatcher)
			    : dispatcher(dispatcher)
			{
				buffer.set_capacity(buffered_elements);
			}
		};

		template <class Element, class Dispatcher>
		struct awaitable_read
		{
			typedef nothing message_type;

			explicit awaitable_read(std::shared_ptr<shared_state<Element, Dispatcher>> shared)
			    : m_shared(std::move(shared))
			{
			}

			void async_wait(function<void(message_type)> callback)
			{
				assert(!m_shared->readable_callback);
				m_shared->readable_callback = std::move(callback);
			}

		private:
			std::shared_ptr<shared_state<Element, Dispatcher>> m_shared;
		};

		template <class Element, class Dispatcher>
		struct awaitable_write
		{
			typedef nothing message_type;

			explicit awaitable_write(std::shared_ptr<shared_state<Element, Dispatcher>> shared)
			    : m_shared(std::move(shared))
			{
			}

			void async_wait(function<void(message_type)> callback)
			{
				assert(!m_shared->writeable_callback);
				m_shared->writeable_callback = std::move(callback);
			}

		private:
			std::shared_ptr<shared_state<Element, Dispatcher>> m_shared;
		};

		template <class Element, class Dispatcher>
		struct async_pipe_sink
		{
			typedef Element element_type;
			typedef success error_type;

			explicit async_pipe_sink(std::shared_ptr<shared_state<Element, Dispatcher>> shared)
			    : m_shared(std::move(shared))
			{
			}

			error_type append(iterator_range<element_type const *> elements)
			{
				std::ptrdiff_t const free = m_shared->buffer.capacity() - m_shared->buffer.size();
				std::ptrdiff_t const copied = (std::min)(elements.size(), free);
				// TODO: this Sink breaks its promise to handle all elements because we use a limited buffer. At the
				// same time we do not really want to enlarge the buffer of the pipe indefinitely. This shows that the
				// Sink
				// is an unsuitable abstraction for the writer's side. We have to use something else, maybe
				// async_write-like.
				m_shared->buffer.insert(m_shared->buffer.end(), elements.begin(), elements.begin() + copied);
				function<void(nothing)> readable_callback =
				    Si::exchange(m_shared->readable_callback, function<void(nothing)>());
				if (!readable_callback)
				{
					return error_type();
				}
				m_shared->dispatcher.post([SILICIUM_CAPTURE_EXPRESSION(readable_callback, std::move(readable_callback))]
				                          {
					                          readable_callback(nothing());
					                      });
				return error_type();
			}

		private:
			std::shared_ptr<shared_state<Element, Dispatcher>> m_shared;
		};

		template <class Element, class Dispatcher>
		struct async_pipe_source
		{
			typedef Element element_type;

			explicit async_pipe_source(std::shared_ptr<shared_state<Element, Dispatcher>> shared)
			    : m_shared(std::move(shared))
			{
			}

			iterator_range<element_type const *> map_next(std::size_t count)
			{
				// TODO: find working semantics for this method
				ignore_unused_variable_warning(count);
				return iterator_range<element_type const *>();
			}

			element_type *copy_next(iterator_range<element_type *> destination)
			{
				element_type *until = destination.begin();
				for (; !m_shared->buffer.empty() && (until != destination.end()); ++until)
				{
					// TODO: better exception safety
					*until = m_shared->buffer.front();
					m_shared->buffer.pop_front();
				}
				if (until != destination.begin())
				{
					// removed something from the buffer. Writing is possible again.
					function<void(nothing)> writeable_callback =
					    Si::exchange(m_shared->writeable_callback, function<void(nothing)>());
					if (writeable_callback)
					{
						m_shared->dispatcher.post(
						    [SILICIUM_CAPTURE_EXPRESSION(writeable_callback, std::move(writeable_callback))]
						    {
							    writeable_callback(nothing());
							});
					}
				}
				return until;
			}

		private:
			std::shared_ptr<shared_state<Element, Dispatcher>> m_shared;
		};
	}

	template <class Element, class Dispatcher>
	async_pipe<Element> make_async_pipe(std::size_t buffered_elements, Dispatcher &dispatcher)
	{
		auto shared = std::make_shared<detail::shared_state<Element, Dispatcher>>(buffered_elements, dispatcher);
		return async_pipe<Element>{
		    async_pipe_reader<Element>{
		        Source<Element>::make_box(detail::async_pipe_source<Element, Dispatcher>(shared)),
		        Awaitable<nothing>::make_box(detail::awaitable_read<Element, Dispatcher>(shared))},
		    async_pipe_writer<Element>{
		        Sink<Element, success>::make_box(detail::async_pipe_sink<Element, Dispatcher>(shared)),
		        Awaitable<nothing>::make_box(detail::awaitable_write<Element, Dispatcher>(shared))}};
	}
}

BOOST_AUTO_TEST_CASE(async_pipe_test)
{
	typedef int message;
	boost::asio::io_service dispatcher;
	Si::async_pipe<message> pipe = Si::make_async_pipe<message>(2u, dispatcher);
	pipe.reader.readable.async_wait([](Si::nothing)
	                                {

		                            });
	pipe.writer.writable.async_wait([](Si::nothing)
	                                {

		                            });
}
#endif
