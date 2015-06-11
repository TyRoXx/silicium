#ifndef SILICIUM_SINK_HPP
#define SILICIUM_SINK_HPP

#include <silicium/trait.hpp>
#include <silicium/iterator_range.hpp>
#include <silicium/memory_range.hpp>
#include <silicium/utility.hpp>
#include <silicium/success.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/filesystem/path.hpp>
#if BOOST_VERSION >= 105300
#	include <boost/utility/string_ref.hpp>
#endif
#if BOOST_VERSION >= 105500
#	include <boost/utility/explicit_operator_bool.hpp>
#endif
#include <boost/container/string.hpp>

namespace Si
{
	template <class Element, class Error = success>
	SILICIUM_TRAIT_WITH_TYPEDEFS(
		Sink,
		typedef Element element_type;
		typedef Error error_type;
		,
		((append, (1, (iterator_range<element_type const *>)), error_type))
	)

	template <class Element, class Error = boost::system::error_code>
	using sink = typename Sink<Element, Error>::interface;

	template <class Element, class Error = boost::system::error_code>
	struct null_sink
	{
		typedef Element element_type;
		typedef Error error_type;

		error_type append(iterator_range<element_type const *> data)
		{
			boost::ignore_unused_variable_warning(data);
			return error_type();
		}
	};

	template <class Element, class Error = boost::system::error_code>
	struct buffer
	{
		typedef Element element_type;
		typedef Error error_type;

		virtual ~buffer()
		{
		}

		virtual iterator_range<element_type *> make_append_space(std::size_t size) = 0;
		virtual error_type flush_append_space() = 0;
	};

	template <class Stream>
	struct error_type
	{
		typedef typename std::decay<Stream>::type clean;
		typedef typename clean::error_type type;
	};

	template <class Sink, class Element>
	auto append(Sink &&out, std::basic_string<Element> const &str)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> typename error_type<Sink>::type
#endif
	{
		return out.append(make_iterator_range(str.data(), str.data() + str.size()));
	}

	template <class Sink, class Element>
	auto append(Sink &&out, boost::container::basic_string<Element> const &str)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> typename error_type<Sink>::type
#endif
	{
		return out.append(make_iterator_range(str.data(), str.data() + str.size()));
	}

#if BOOST_VERSION >= 105300
	template <class Sink, class Element, class CharTraits>
	auto append(Sink &&out, boost::basic_string_ref<Element, CharTraits> const &str)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> typename error_type<Sink>::type
#endif
	{
		return out.append(make_memory_range(str));
	}
#endif

	template <class Sink, class Element>
	auto append(Sink &&out, Element const *c_str)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> typename error_type<Sink>::type
#endif
	{
		return out.append(make_iterator_range(c_str, c_str + std::char_traits<Element>::length(c_str)));
	}

	template <class Sink, class Element>
	auto append(Sink &&out, Element const &single)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> typename error_type<Sink>::type
#endif
	{
		return out.append(make_iterator_range(&single, &single + 1));
	}

	template <class Sink>
	auto append(Sink &&out, iterator_range<typename std::decay<Sink>::type::element_type const *> const &elements)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> typename error_type<Sink>::type
#endif
	{
		return out.append(elements);
	}
}

#endif
