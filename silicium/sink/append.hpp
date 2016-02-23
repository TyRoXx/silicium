#ifndef SILICIUM_SINK_APPEND_HPP
#define SILICIUM_SINK_APPEND_HPP

#include <silicium/sink/sink.hpp>
#include <silicium/memory_range.hpp>
#if BOOST_VERSION >= 105300
#include <boost/utility/string_ref.hpp>
#endif
#include <boost/container/string.hpp>

namespace Si
{
	template <class Sink, class Element>
	typename boost::disable_if<
	    std::is_same<typename std::decay<Sink>::type::element_type,
	                 std::basic_string<Element>>,
	    typename error_type<Sink>::type>::type
	append(Sink &&out, std::basic_string<Element> const &str)
	{
		return out.append(
		    make_iterator_range(str.data(), str.data() + str.size()));
	}

	template <class Sink, class Element>
	typename boost::disable_if<
	    std::is_same<typename std::decay<Sink>::type::element_type,
	                 boost::container::basic_string<Element>>,
	    typename error_type<Sink>::type>::type
	append(Sink &&out, boost::container::basic_string<Element> const &str)
	{
		return out.append(
		    make_iterator_range(str.data(), str.data() + str.size()));
	}

#if BOOST_VERSION >= 105300
	template <class Sink, class Element, class CharTraits>
	auto append(Sink &&out,
	            boost::basic_string_ref<Element, CharTraits> const &str)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
	    -> typename error_type<Sink>::type
#endif
	{
		return out.append(make_memory_range(str));
	}
#endif

	template <class Sink, class Element>
	typename std::enable_if<
	    std::is_same<Element,
	                 typename std::decay<Sink>::type::element_type>::value,
	    typename error_type<Sink>::type>::type
	append(Sink &&out, Element const *c_str)
	{
		return out.append(make_iterator_range(
		    c_str, c_str + std::char_traits<Element>::length(c_str)));
	}

	template <class Sink>
	auto append(Sink &&out,
	            typename std::decay<Sink>::type::element_type const &single)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
	    -> typename error_type<Sink>::type
#endif
	{
		return out.append(make_iterator_range(&single, &single + 1));
	}

	template <class Sink>
	auto append(Sink &&out,
	            iterator_range<typename std::decay<
	                Sink>::type::element_type const *> const &elements)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
	    -> typename error_type<Sink>::type
#endif
	{
		return out.append(elements);
	}

	template <class Sink>
	auto append_range(Sink &&out,
	                  iterator_range<typename std::decay<
	                      Sink>::type::element_type const *> const &elements)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
	    -> typename error_type<Sink>::type
#endif
	{
		return out.append(elements);
	}
}

#endif
