#pragma once

#include <silicium/bounded_int.hpp>

namespace Si
{
    template <class Element, class StrongInt>
    struct memory_size
    {
        StrongInt value;
    };

    template <class Element, class StrongInt>
    constexpr auto make_memory_size(StrongInt value)
    {
        return memory_size<Element, StrongInt>{value};
    }

    template <class Element, class StrongInt>
    constexpr auto as_chars(memory_size<Element, StrongInt> size)
    {
        return make_memory_size<char>(size.value *
                                      literal<std::size_t, sizeof(Element)>());
    }
}
