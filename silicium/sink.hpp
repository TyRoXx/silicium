#ifndef SILICIUM_SINK_HPP
#define SILICIUM_SINK_HPP

#include <boost/range/iterator_range.hpp>
#include <boost/range/algorithm/copy.hpp>

namespace Si
{
	template <class Element>
	struct sink
	{
		virtual ~sink()
		{
		}

		virtual boost::iterator_range<Element *> make_append_space(std::size_t size) = 0;
		virtual void flush_append_space() = 0;
		virtual void append(boost::iterator_range<Element const *> data) = 0;
	};

	template <class Element>
	void commit(sink<Element> &destination, std::size_t count)
	{
		destination.make_append_space(count);
		destination.flush_append_space();
	}

	template <class Element, class Buffer = std::array<Element, ((1U << 13U) / sizeof(Element))>>
	struct buffering_sink : sink<Element>
	{
		explicit buffering_sink(sink<Element> &destination, Buffer buffer = Buffer())
			: m_destination(destination)
			, m_fallback_buffer(std::move(buffer))
		{
		}

		boost::iterator_range<Element *> make_append_space(std::size_t size) override
		{
			auto first_try = m_destination.make_append_space(size);
			if (!first_try.empty())
			{
				auto const copied = (std::min)(static_cast<std::ptrdiff_t>(m_buffer_used), first_try.size());
				std::copy(m_fallback_buffer.begin(), m_fallback_buffer.begin() + copied, first_try.begin());
				m_buffer_used = 0;
				return first_try;
			}
			m_buffer_used = (std::min)(size, m_fallback_buffer.size());
			return boost::make_iterator_range(m_fallback_buffer.data(), m_fallback_buffer.data() + m_buffer_used);
		}

		void flush_append_space() override
		{
			if (m_buffer_used)
			{
				m_destination.append(boost::make_iterator_range(m_fallback_buffer.data(), m_fallback_buffer.data() + m_buffer_used));
				m_buffer_used = 0;
			}
			else
			{
				m_destination.flush_append_space();
			}
		}

		void append(boost::iterator_range<Element const *> data) override
		{
			m_destination.append(data);
			m_buffer_used = 0;
		}

	private:

		sink<Element> &m_destination;
		Buffer m_fallback_buffer;
		std::size_t m_buffer_used = 0;
	};

	template <class Element, class OutputIterator>
	struct iterator_sink : sink<Element>
	{
		explicit iterator_sink(OutputIterator out)
			: m_out(std::move(out))
		{
		}

		virtual boost::iterator_range<Element *> make_append_space(std::size_t) override
		{
			return {};
		}

		virtual void flush_append_space() override
		{
		}

		virtual void append(boost::iterator_range<Element const *> data) override
		{
			boost::range::copy(data, m_out);
		}

	private:

		OutputIterator m_out;
	};

	template <class Element, class OutputIterator>
	auto make_iterator_sink(OutputIterator out)
	{
		return iterator_sink<Element, typename std::decay<OutputIterator>::type>(std::move(out));
	}

	template <class Container>
	auto make_container_sink(Container &destination)
	{
		return make_iterator_sink<typename Container::value_type>(std::back_inserter(destination));
	}
}

#endif
