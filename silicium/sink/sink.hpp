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
		typedef Error error_type;

		virtual ~sink()
		{
		}

		virtual error_type append(boost::iterator_range<element_type const *> data) = 0;
	};

	template <class Element, class Error = boost::system::error_code>
	struct buffer
	{
		typedef Element element_type;
		typedef Error error_type;

		virtual ~buffer()
		{
		}

		virtual boost::iterator_range<element_type *> make_append_space(std::size_t size) = 0;
		virtual error_type flush_append_space() = 0;
	};

	template <class Element, class Error>
	Error commit(buffer<Element, Error> &destination, std::size_t count)
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

	template <class Next, class Error = typename Next::error_type, class Buffer = std::array<typename Next::element_type, ((1U << 13U) / sizeof(typename Next::element_type))>>
	struct buffering_sink SILICIUM_FINAL
		: sink  <typename Next::element_type, typename Next::error_type>
		, buffer<typename Next::element_type, typename Next::error_type>
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

		boost::iterator_range<element_type *> make_append_space(std::size_t size) SILICIUM_OVERRIDE
		{
			m_buffer_used = (std::min)(size, m_fallback_buffer.size());
			return boost::make_iterator_range(m_fallback_buffer.data(), m_fallback_buffer.data() + m_buffer_used);
		}

		Error flush_append_space() SILICIUM_OVERRIDE
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

		Error append(boost::iterator_range<element_type const *> data) SILICIUM_OVERRIDE
		{
			if (static_cast<size_t>(data.size()) <= (m_fallback_buffer.size() - m_buffer_used))
			{
				boost::range::copy(data, m_fallback_buffer.begin() + m_buffer_used);
				m_buffer_used += data.size();
				return default_construct<Error>();
			}

			return then(
				[this] { return flush(); },
				[this, &data] { return m_destination.append(data); }
			);
		}

		Error flush()
		{
			return then(
				[this] { return m_destination.append(boost::make_iterator_range(m_fallback_buffer.data(), m_fallback_buffer.data() + m_buffer_used)); },
				[this] { m_buffer_used = 0; return Error(); }
			);
		}

	private:

		Next m_destination;
		Buffer m_fallback_buffer;
		std::size_t m_buffer_used;
	};

	template <class Next>
	auto make_buffering_sink(Next &&next)
	{
		return buffering_sink<typename std::decay<Next>::type>(std::forward<Next>(next));
	}

	struct ostream_ref_sink SILICIUM_FINAL : sink<char, void>
	{
		ostream_ref_sink()
			: m_file(nullptr)
		{
		}

		explicit ostream_ref_sink(std::ostream &file)
		   : m_file(&file)
		{
		}

		virtual void append(boost::iterator_range<char const *> data) SILICIUM_OVERRIDE
		{
			assert(m_file);
			m_file->write(data.begin(), data.size());
		}

	private:

		std::ostream *m_file;
	};

	struct ostream_sink SILICIUM_FINAL : sink<char, boost::system::error_code>
	{
		//unique_ptr to make ostreams movable
		explicit ostream_sink(std::unique_ptr<std::ostream> file)
			: m_file(std::move(file))
		{
			m_file->exceptions(std::ios::failbit | std::ios::badbit);
		}

		virtual boost::system::error_code append(boost::iterator_range<char const *> data) SILICIUM_OVERRIDE
		{
			m_file->write(data.begin(), data.size());
			return {};
		}

	private:

		std::unique_ptr<std::ostream> m_file;
	};

	inline std::unique_ptr<sink<char, boost::system::error_code>> make_file_sink(boost::filesystem::path const &name)
	{
		std::unique_ptr<std::ostream> file(new std::ofstream(name.string(), std::ios::binary));
		if (!*file)
		{
			throw std::runtime_error("Cannot open file for writing: " + name.string());
		}
		return std::unique_ptr<sink<char, boost::system::error_code>>(new ostream_sink(std::move(file)));
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
