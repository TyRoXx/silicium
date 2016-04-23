#ifndef SILICIUM_SOURCE_HPP
#define SILICIUM_SOURCE_HPP

#include <silicium/trait.hpp>
#include <silicium/config.hpp>
#include <silicium/iterator_range.hpp>
#include <silicium/optional.hpp>
#include <boost/cstdint.hpp>
#include <vector>

namespace Si
{
    template <class Element>
    SILICIUM_TRAIT_WITH_TYPEDEFS(
        Source, typedef Element element_type;
        ,
        ((map_next, (1, (std::size_t)), iterator_range<element_type const *>))(
            (copy_next, (1, (iterator_range<element_type *>)), element_type *)))

#if SILICIUM_COMPILER_HAS_USING
    template <class Element>
    using source = typename Source<Element>::interface;
#endif

    template <class Source>
    optional<typename Source::element_type> get(Source &from)
    {
        typename Source::element_type result;
        if (&result ==
            from.copy_next(Si::make_iterator_range(&result, &result + 1)))
        {
            return Si::none;
        }
        return std::move(result);
    }

    template <class Container>
    auto data(Container &container) -> decltype(&container[0])
    {
        return container.empty() ? nullptr : &container[0];
    }

    template <class Sequence, class Source>
    auto take(Source &from, std::size_t count) -> Sequence
    {
        Sequence taken;
        taken.resize(count);
        auto end = from.copy_next(
            make_iterator_range(data(taken), data(taken) + taken.size()));
        taken.resize(std::distance(data(taken), end));
        return taken;
    }
}

#endif
