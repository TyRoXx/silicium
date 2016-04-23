#ifndef SILICIUM_OWNING_SOURCE_HPP
#define SILICIUM_OWNING_SOURCE_HPP

#include <silicium/source/source.hpp>
#include <iterator>
#include <array>
#include <boost/array.hpp>

namespace Si
{
    namespace detail
    {
        template <class Range>
        struct owning_source_indirect
        {
            typedef typename Range::iterator iterator;
            typedef typename std::iterator_traits<iterator>::value_type
                element_type;

            explicit owning_source_indirect(Range owned)
                : m_owned(std::move(owned))
                , m_next(m_owned.begin())
            {
            }

            iterator_range<element_type const *> map_next(std::size_t size)
            {
                ignore_unused_variable_warning(size);
                return {};
            }

            element_type *copy_next(iterator_range<element_type *> destination)
            {
                element_type *i = destination.begin();
                for (; i != destination.end() && m_next != m_owned.end();
                     ++i, ++m_next)
                {
                    *i = *m_next;
                }
                return i;
            }

        private:
            Range m_owned;
            iterator m_next;
        };

        template <class Range>
        struct owning_source_direct
        {
            typedef typename std::iterator_traits<
                typename Range::iterator>::value_type element_type;

            explicit owning_source_direct(Range owned)
                : m_owned(std::move(owned))
                , m_next(0)
            {
            }

            iterator_range<element_type const *> map_next(std::size_t size)
            {
                std::size_t const mapped =
                    std::min(m_owned.size() - m_next, size);
                m_next += mapped;
                return Si::make_contiguous_range(
                    m_owned.begin() + m_next - mapped,
                    m_owned.begin() + m_next);
            }

            element_type *copy_next(iterator_range<element_type *> destination)
            {
                element_type *i = destination.begin();
                for (; i != destination.end() && m_next < m_owned.size();
                     ++i, ++m_next)
                {
                    *i = m_owned[m_next];
                }
                return i;
            }

        private:
            Range m_owned;
            std::size_t m_next;
        };
    }

    template <class Range>
    struct is_direct_range : std::false_type
    {
    };

    template <class T, std::size_t N>
    struct is_direct_range<std::array<T, N>> : std::true_type
    {
    };

    template <class T, std::size_t N>
    struct is_direct_range<boost::array<T, N>> : std::true_type
    {
    };

    template <class Range>
    struct owning_source
        : std::conditional<is_direct_range<Range>::value,
                           detail::owning_source_direct<Range>,
                           detail::owning_source_indirect<Range>>::type
    {
        explicit owning_source(Range owned)
            : std::conditional<
                  is_direct_range<Range>::value,
                  detail::owning_source_direct<Range>,
                  detail::owning_source_indirect<Range>>::type(std::move(owned))
        {
        }
    };

    template <class Range>
    owning_source<typename std::decay<Range>::type>
    make_owning_source(Range &&owned)
    {
        return owning_source<typename std::decay<Range>::type>(
            std::forward<Range>(owned));
    }
}

#endif
