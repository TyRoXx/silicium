#ifndef SILICIUM_MEMORY_SOURCE_HPP
#define SILICIUM_MEMORY_SOURCE_HPP

#include <silicium/source/source.hpp>
#include <boost/concept_check.hpp>
#include <vector>
#include <string>
#include <array>

namespace Si
{
    template <class Element>
    struct memory_source
    {
        typedef Element element_type;

        memory_source()
        {
        }

        explicit memory_source(iterator_range<Element const *> elements)
            : m_elements(std::move(elements))
        {
        }

        iterator_range<Element const *> map_next(std::size_t size)
        {
            boost::ignore_unused_variable_warning(size);
            return m_elements;
        }

        Element *copy_next(iterator_range<Element *> destination)
        {
            while (!m_elements.empty() && !destination.empty())
            {
                destination.front() = m_elements.front();
                destination.pop_front();
                m_elements.pop_front();
            }
            return destination.begin();
        }

    private:
        iterator_range<Element const *> m_elements;
    };

    template <class Element>
    struct mutable_memory_source
    {
        typedef Element element_type;

        mutable_memory_source()
        {
        }

        explicit mutable_memory_source(iterator_range<Element *> elements)
            : m_elements(std::move(elements))
        {
        }

        iterator_range<Element const *> map_next(std::size_t size)
        {
            boost::ignore_unused_variable_warning(size);
            return {};
        }

        Element *copy_next(iterator_range<Element *> destination)
        {
            while (!m_elements.empty() && !destination.empty())
            {
                destination.front() = std::move(m_elements.front());
                destination.pop_front();
                m_elements.pop_front();
            }
            return destination.begin();
        }

    private:
        iterator_range<Element *> m_elements;
    };

    template <class Element>
    memory_source<Element>
    make_container_source(std::vector<Element> const &container)
    {
        return memory_source<Element>(iterator_range<Element const *>(
            container.data(), container.data() + container.size()));
    }

    template <class Element, std::size_t N>
    memory_source<Element>
    make_container_source(std::array<Element, N> const &container)
    {
        return memory_source<Element>(make_iterator_range(
            container.data(), container.data() + container.size()));
    }

    template <class Element, std::size_t N>
    mutable_memory_source<Element>
    make_container_source(std::array<Element, N> &&container)
    {
        return mutable_memory_source<Element>(make_iterator_range(
            container.data(), container.data() + container.size()));
    }

    template <class Element>
    memory_source<Element>
    make_container_source(std::basic_string<Element> const &container)
    {
        return memory_source<Element>(make_iterator_range(
            container.data(), container.data() + container.size()));
    }

    template <class Element>
    memory_source<Element> make_c_str_source(Element const *c_str)
    {
        return memory_source<Element>(make_iterator_range(
            c_str, c_str + std::char_traits<Element>::length(c_str)));
    }
}

#endif
