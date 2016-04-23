#ifndef SILICIUM_CONSTANT_OBSERVABLE_HPP
#define SILICIUM_CONSTANT_OBSERVABLE_HPP

#include <silicium/observable/observable.hpp>
#include <silicium/config.hpp>

namespace Si
{
    template <class Element>
    struct constant_observable
    {
        typedef Element element_type;

        constant_observable()
        {
        }

        explicit constant_observable(Element value)
            : m_value(std::move(value))
        {
        }

        template <class OtherElement>
        constant_observable(constant_observable<OtherElement> &&other)
            : m_value(other.constant())
        {
        }

        template <class ElementObserver>
        void async_get_one(ElementObserver &&receiver) const
        {
            std::forward<ElementObserver>(receiver).got_element(m_value);
        }

        Element const &constant() const BOOST_NOEXCEPT
        {
            return m_value;
        }

    private:
        Element m_value;
    };

    template <class Element>
    auto make_constant_observable(Element &&value)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
        -> constant_observable<typename std::decay<Element>::type>
#endif
    {
        return constant_observable<typename std::decay<Element>::type>(
            std::forward<Element>(value));
    }
}

#endif
