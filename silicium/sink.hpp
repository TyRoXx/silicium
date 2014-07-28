#ifndef SILICIUM_SINK_HPP
#define SILICIUM_SINK_HPP

#include <silicium/override.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/filesystem/path.hpp>
#include <ostream>
#include <array>
#include <memory>

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
	struct flushable_sink : sink<Element>
	{
		virtual void flush() = 0;
	};

	template <class Element>
	void commit(sink<Element> &destination, std::size_t count)
	{
		destination.make_append_space(count);
		destination.flush_append_space();
	}

	template <class Element, class Buffer = std::array<Element, ((1U << 13U) / sizeof(Element))>>
	struct buffering_sink SILICIUM_FINAL : flushable_sink<Element>
	{
		explicit buffering_sink(sink<Element> &destination, Buffer buffer = Buffer())
			: m_destination(destination)
			, m_fallback_buffer(std::move(buffer))
			, m_buffer_used(0)
		{
		}

		boost::iterator_range<Element *> make_append_space(std::size_t size) SILICIUM_OVERRIDE
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

		void flush_append_space() SILICIUM_OVERRIDE
		{
			if (m_buffer_used)
			{
				flush();
			}
			else
			{
				m_destination.flush_append_space();
			}
		}

		void append(boost::iterator_range<Element const *> data) SILICIUM_OVERRIDE
		{
			if (data.size() <= (m_fallback_buffer.size() - m_buffer_used))
			{
				boost::range::copy(data, m_fallback_buffer.begin() + m_buffer_used);
				m_buffer_used += data.size();
				return;
			}

			flush();
			m_destination.append(data);
		}

		void flush() SILICIUM_OVERRIDE
		{
			m_destination.append(boost::make_iterator_range(m_fallback_buffer.data(), m_fallback_buffer.data() + m_buffer_used));
			m_buffer_used = 0;
		}

	private:

		sink<Element> &m_destination;
		Buffer m_fallback_buffer;
		std::size_t m_buffer_used;
	};

	template <class Element, class OutputIterator>
	struct iterator_sink SILICIUM_FINAL : sink<Element>
	{
		explicit iterator_sink(OutputIterator out)
			: m_out(std::move(out))
		{
		}

		virtual boost::iterator_range<Element *> make_append_space(std::size_t) SILICIUM_OVERRIDE
		{
			return {};
		}

		virtual void flush_append_space() SILICIUM_OVERRIDE
		{
		}

		virtual void append(boost::iterator_range<Element const *> data) SILICIUM_OVERRIDE
		{
			boost::range::copy(data, m_out);
		}

	private:

		OutputIterator m_out;
	};

	template <class Element, class OutputIterator>
	auto make_iterator_sink(OutputIterator out)
		-> iterator_sink<Element, typename std::decay<OutputIterator>::type>
	{
		return iterator_sink<Element, typename std::decay<OutputIterator>::type>(std::move(out));
	}

	template <class Container>
	auto make_container_sink(Container &destination)
		-> iterator_sink<typename Container::value_type, std::back_insert_iterator<Container>>
	{
		return make_iterator_sink<typename Container::value_type>(std::back_inserter(destination));
	}

	struct ostream_ref_sink SILICIUM_FINAL : flushable_sink<char>
	{
		ostream_ref_sink()
		{
		}

		explicit ostream_ref_sink(std::ostream &file)
		   : m_file(&file)
		{
		}

		virtual boost::iterator_range<char *> make_append_space(std::size_t) SILICIUM_OVERRIDE
		{
		   return {};
		}

		virtual void flush_append_space() SILICIUM_OVERRIDE
		{
		}

		virtual void append(boost::iterator_range<char const *> data) SILICIUM_OVERRIDE
		{
			assert(m_file);
			m_file->write(data.begin(), data.size());
		}

		virtual void flush() SILICIUM_OVERRIDE
		{
			assert(m_file);
			m_file->flush();
		}

	private:

		std::ostream *m_file = nullptr;
	};

	struct ostream_sink SILICIUM_FINAL : flushable_sink<char>
	{
		//unique_ptr to make ostreams movable
		explicit ostream_sink(std::unique_ptr<std::ostream> file)
			: m_file(std::move(file))
		{
			m_file->exceptions(std::ios::failbit | std::ios::badbit);
		}

		virtual boost::iterator_range<char *> make_append_space(std::size_t) SILICIUM_OVERRIDE
		{
			return {};
		}

		virtual void flush_append_space() SILICIUM_OVERRIDE
		{
		}

		virtual void append(boost::iterator_range<char const *> data) SILICIUM_OVERRIDE
		{
			m_file->write(data.begin(), data.size());
		}

		virtual void flush() SILICIUM_OVERRIDE
		{
			m_file->flush();
		}

	private:

		std::unique_ptr<std::ostream> m_file;
	};

	std::unique_ptr<flushable_sink<char>> make_file_sink(boost::filesystem::path const &name);

	template <class Element>
	struct auto_flush_sink SILICIUM_FINAL : sink<Element>
	{
		auto_flush_sink()
			: m_next(nullptr)
		{
		}

		explicit auto_flush_sink(flushable_sink<Element> &next)
			: m_next(&next)
		{
		}

		virtual boost::iterator_range<char *> make_append_space(std::size_t size) SILICIUM_OVERRIDE
		{
			assert(m_next);
			return m_next->make_append_space(size);
		}

		virtual void flush_append_space() SILICIUM_OVERRIDE
		{
			assert(m_next);
			m_next->flush_append_space();
			m_next->flush();
		}

		virtual void append(boost::iterator_range<char const *> data) SILICIUM_OVERRIDE
		{
			assert(m_next);
			m_next->append(data);
			m_next->flush();
		}

	private:

		flushable_sink<Element> *m_next;
	};

	template <class Element>
	auto_flush_sink<Element> make_auto_flush_sink(flushable_sink<Element> &next)
	{
		return auto_flush_sink<Element>(next);
	}

	template <class Element>
	void append(Si::sink<Element> &out, std::basic_string<Element> const &str)
	{
		out.append(boost::make_iterator_range(str.data(), str.data() + str.size()));
	}

	template <class Element>
	void append(Si::sink<Element> &out, Element const *c_str)
	{
		out.append(boost::make_iterator_range(c_str, c_str + std::char_traits<Element>::length(c_str)));
	}

	template <class Element>
	void append(Si::sink<Element> &out, Element const &single)
	{
		out.append(boost::make_iterator_range(&single, &single + 1));
	}
}

#endif
