#ifndef SILICIUM_BUFFERING_SINK_HPP
#define SILICIUM_BUFFERING_SINK_HPP

#include <silicium/sink/sink.hpp>
#include <silicium/then.hpp>
#include <array>

#define SILICIUM_HAS_BUFFERING_SINK SILICIUM_HAS_THEN

namespace Si
{
#if SILICIUM_HAS_BUFFERING_SINK
	template <class Next, class Error = typename Next::error_type,
	          class Buffer =
	              std::array<typename Next::element_type, ((1U << 13U) / sizeof(typename Next::element_type))>>
	struct buffering_sink SILICIUM_FINAL
	{
		typedef typename Next::element_type element_type;
		typedef typename Next::error_type error_type;

		buffering_sink()
		{
		}

		explicit buffering_sink(Next destination, Buffer buffer = Buffer())
		    : m_destination(std::move(destination))
		    , m_fallback_buffer(std::move(buffer))
		    , m_buffer_used(0)
		{
		}

		iterator_range<element_type *> make_append_space(std::size_t size)
		{
			m_buffer_used = (std::min)(size, m_fallback_buffer.size());
			return make_iterator_range(m_fallback_buffer.data(), m_fallback_buffer.data() + m_buffer_used);
		}

		Error flush_append_space()
		{
			if (m_buffer_used)
			{
				return flush();
			}
			else
			{
				return Error();
			}
		}

		Error append(iterator_range<element_type const *> data)
		{
			if (static_cast<size_t>(data.size()) <= (m_fallback_buffer.size() - m_buffer_used))
			{
				boost::range::copy(data, m_fallback_buffer.begin() + m_buffer_used);
				m_buffer_used += static_cast<size_t>(data.size());
				return default_construct<Error>();
			}

			return then(
			    [this]
			    {
				    return this->flush();
				},
			    [this, &data]
			    {
				    return m_destination.append(data);
				});
		}

		Error flush()
		{
			return then(
			    [this]
			    {
				    return m_destination.append(
				        make_iterator_range(m_fallback_buffer.data(), m_fallback_buffer.data() + m_buffer_used));
				},
			    [this]
			    {
				    m_buffer_used = 0;
				    return Error();
				});
		}

	private:
		Next m_destination;
		Buffer m_fallback_buffer;
		std::size_t m_buffer_used;
	};

	template <class Next>
	auto make_buffering_sink(Next &&next)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
	    -> buffering_sink<typename std::decay<Next>::type>
#endif
	{
		return buffering_sink<typename std::decay<Next>::type>(std::forward<Next>(next));
	}
#endif
}

#endif
