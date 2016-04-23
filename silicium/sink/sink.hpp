#ifndef SILICIUM_SINK_HPP
#define SILICIUM_SINK_HPP

#include <silicium/trait.hpp>
#include <silicium/iterator_range.hpp>
#include <silicium/success.hpp>
#include <boost/system/error_code.hpp>

namespace Si
{
    template <class Element, class Error = success>
    SILICIUM_TRAIT_WITH_TYPEDEFS(
        Sink, typedef Element element_type; typedef Error error_type;
        , ((append, (1, (iterator_range<element_type const *>)), error_type)))

#if SILICIUM_COMPILER_HAS_USING
    template <class Element, class Error = boost::system::error_code>
    using sink = typename Sink<Element, Error>::interface;
#endif

    template <class Element, class Error = boost::system::error_code>
    struct null_sink
    {
        typedef Element element_type;
        typedef Error error_type;

        error_type append(iterator_range<element_type const *> data)
        {
            ignore_unused_variable_warning(data);
            return error_type();
        }
    };

    template <class Stream>
    struct error_type
    {
        typedef typename std::decay<Stream>::type clean;
        typedef typename clean::error_type type;
    };
}

#endif
