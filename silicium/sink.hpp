#ifndef SILICIUM_SINK_HPP
#define SILICIUM_SINK_HPP

#include <silicium/override.hpp>
#include <silicium/config.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/container/string.hpp>
#include <fstream>
#include <array>
#include <memory>

namespace Si
{
	template <class Element, class Error = boost::system::error_code>
	struct sink
	{
		typedef Element element_type;

		virtual ~sink()
		{
		}

		virtual boost::iterator_range<Element *> make_append_space(std::size_t size) = 0;
		virtual Error flush_append_space() = 0;
		virtual Error append(boost::iterator_range<Element const *> data) = 0;
	};

	template <class Element, class Error>
	struct flushable_sink : sink<Element, Error>
	{
		typedef Element element_type;

		virtual Error flush() = 0;
	};

	template <class Element, class Error>
	Error commit(sink<Element, Error> &destination, std::size_t count)
	{
		destination.make_append_space(count);
		return destination.flush_append_space();
	}

	namespace detail
	{
		template <class>
		void default_construct(std::true_type)
		{
		}

		template <class T>
		T default_construct(std::false_type)
		{
			return T{};
		}
	}

	template <class T>
	T default_construct()
	{
		return detail::default_construct<T>(std::is_same<T, void>());
	}

	namespace detail
	{
		template <class Error>
		struct then_impl
		{
			Error operator()() const
			{
				return Error();
			}

			template <class First, class ...Tail>
			Error operator()(First &&first, Tail &&...tail) const
			{
				auto error = std::forward<First>(first)();
				if (error)
				{
					return error;
				}
				return (*this)(std::forward<Tail>(tail)...);
			}
		};

		template <>
		struct then_impl<void>
		{
			void operator()() const
			{
			}

			template <class First, class ...Tail>
			void operator()(First &&first, Tail &&...tail) const
			{
				std::forward<First>(first)();
				return (*this)(std::forward<Tail>(tail)...);
			}
		};
	}

	template <class First, class ...Sequence>
	auto then(First &&first, Sequence &&...actions)
		-> decltype(std::forward<First>(first)())
	{
		typedef decltype(std::forward<First>(first)()) result_type;
		return detail::then_impl<result_type>()(std::forward<First>(first), std::forward<Sequence>(actions)...);
	}

	template <class Element, class Error, class Buffer = std::array<Element, ((1U << 13U) / sizeof(Element))>>
	struct buffering_sink SILICIUM_FINAL : flushable_sink<Element, Error>
	{
		typedef Element element_type;

		buffering_sink()
			: m_destination(nullptr)
		{
		}

		explicit buffering_sink(sink<Element, Error> &destination, Buffer buffer = Buffer())
			: m_destination(&destination)
			, m_fallback_buffer(std::move(buffer))
			, m_buffer_used(0)
		{
		}

		boost::iterator_range<Element *> make_append_space(std::size_t size) SILICIUM_OVERRIDE
		{
			assert(m_destination);
			auto first_try = m_destination->make_append_space(size);
			if (!first_try.empty())
			{
				auto const copied = (std::min<std::ptrdiff_t>)(static_cast<std::ptrdiff_t>(m_buffer_used), first_try.size());
				std::copy(m_fallback_buffer.begin(), m_fallback_buffer.begin() + copied, first_try.begin());
				m_buffer_used = 0;
				return first_try;
			}
			m_buffer_used = (std::min)(size, m_fallback_buffer.size());
			return boost::make_iterator_range(m_fallback_buffer.data(), m_fallback_buffer.data() + m_buffer_used);
		}

		Error flush_append_space() SILICIUM_OVERRIDE
		{
			assert(m_destination);
			if (m_buffer_used)
			{
				return flush();
			}
			else
			{
				return m_destination->flush_append_space();
			}
		}

		Error append(boost::iterator_range<Element const *> data) SILICIUM_OVERRIDE
		{
			assert(m_destination);
			if (static_cast<size_t>(data.size()) <= (m_fallback_buffer.size() - m_buffer_used))
			{
				boost::range::copy(data, m_fallback_buffer.begin() + m_buffer_used);
				m_buffer_used += data.size();
				return default_construct<Error>();
			}

			return then(
				[this] { return flush(); },
				[this, &data] { return m_destination->append(data); }
			);
		}

		Error flush() SILICIUM_OVERRIDE
		{
			assert(m_destination);
			return then(
				[this] { return m_destination->append(boost::make_iterator_range(m_fallback_buffer.data(), m_fallback_buffer.data() + m_buffer_used)); },
				[this] { m_buffer_used = 0; return Error(); }
			);
		}

	private:

		sink<Element, Error> *m_destination;
		Buffer m_fallback_buffer;
		std::size_t m_buffer_used;
	};

	template <class Element, class OutputIterator>
	struct iterator_sink SILICIUM_FINAL : sink<Element, void>
	{
		typedef Element element_type;

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

	struct ostream_ref_sink SILICIUM_FINAL : flushable_sink<char, void>
	{
		ostream_ref_sink()
			: m_file(nullptr)
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

		std::ostream *m_file;
	};

	struct ostream_sink SILICIUM_FINAL : flushable_sink<char, boost::system::error_code>
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

		virtual boost::system::error_code flush_append_space() SILICIUM_OVERRIDE
		{
			return {};
		}

		virtual boost::system::error_code append(boost::iterator_range<char const *> data) SILICIUM_OVERRIDE
		{
			m_file->write(data.begin(), data.size());
			return {};
		}

		virtual boost::system::error_code flush() SILICIUM_OVERRIDE
		{
			m_file->flush();
			return {};
		}

	private:

		std::unique_ptr<std::ostream> m_file;
	};

	inline std::unique_ptr<flushable_sink<char, boost::system::error_code>> make_file_sink(boost::filesystem::path const &name)
	{
		std::unique_ptr<std::ostream> file(new std::ofstream(name.string(), std::ios::binary));
		if (!*file)
		{
			throw std::runtime_error("Cannot open file for writing: " + name.string());
		}
		return std::unique_ptr<flushable_sink<char, boost::system::error_code>>(new ostream_sink(std::move(file)));
	}

	template <class Element, class Error>
	struct auto_flush_sink SILICIUM_FINAL : sink<Element, Error>
	{
		auto_flush_sink()
			: m_next(nullptr)
		{
		}

		explicit auto_flush_sink(flushable_sink<Element, Error> &next)
			: m_next(&next)
		{
		}

		virtual boost::iterator_range<char *> make_append_space(std::size_t size) SILICIUM_OVERRIDE
		{
			assert(m_next);
			return m_next->make_append_space(size);
		}

		virtual Error flush_append_space() SILICIUM_OVERRIDE
		{
			assert(m_next);
			return then(
				[this] { return m_next->flush_append_space(); },
				[this] { return m_next->flush(); }
			);
		}

		virtual Error append(boost::iterator_range<char const *> data) SILICIUM_OVERRIDE
		{
			assert(m_next);
			return then(
				[this, &data] { return m_next->append(data); },
				[this] { return m_next->flush(); }
			);
		}

	private:

		flushable_sink<Element, Error> *m_next;
	};

	template <class Element, class Error>
	auto_flush_sink<Element, Error> make_auto_flush_sink(flushable_sink<Element, Error> &next)
	{
		return auto_flush_sink<Element, Error>(next);
	}

	template <class Element, class Error>
	Error append(Si::sink<Element, Error> &out, std::basic_string<Element> const &str)
	{
		return out.append(boost::make_iterator_range(str.data(), str.data() + str.size()));
	}

	template <class Element, class Error>
	Error append(Si::sink<Element, Error> &out, boost::container::basic_string<Element> const &str)
	{
		return out.append(boost::make_iterator_range(str.data(), str.data() + str.size()));
	}

	template <class Element, class Error>
	Error append(Si::sink<Element, Error> &out, Element const *c_str)
	{
		return out.append(boost::make_iterator_range(c_str, c_str + std::char_traits<Element>::length(c_str)));
	}

	template <class Element, class Error>
	Error append(Si::sink<Element, Error> &out, Element const &single)
	{
		return out.append(boost::make_iterator_range(&single, &single + 1));
	}
}

#endif
