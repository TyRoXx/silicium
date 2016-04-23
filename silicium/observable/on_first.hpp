#ifndef SILICIUM_OBSERVABLE_ON_FIRST_HPP
#define SILICIUM_OBSERVABLE_ON_FIRST_HPP

#include <silicium/observable/observer.hpp>
#include <silicium/optional.hpp>

namespace Si
{
    template <class Input, class Handler>
    struct on_first_observable : observer<typename Input::element_type>
    {
        on_first_observable()
        {
        }

        on_first_observable(Input input, Handler handler)
            : m_input(std::move(input))
            , m_handler(std::move(handler))
        {
        }

        void start()
        {
            m_input.async_get_one(observe_by_ref(*this));
        }

    private:
        Input m_input;
        Handler m_handler;

        virtual void
        got_element(typename Input::element_type value) SILICIUM_OVERRIDE
        {
            std::move(m_handler)(std::move(value));
        }

        virtual void ended() SILICIUM_OVERRIDE
        {
            std::move(m_handler)(optional<typename Input::element_type>());
        }
    };

    template <class Input, class Handler>
    auto on_first(Input &&input, Handler &&handle_element)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
        -> on_first_observable<typename std::decay<Input>::type,
                               typename std::decay<Handler>::type>
#endif
    {
        return on_first_observable<typename std::decay<Input>::type,
                                   typename std::decay<Handler>::type>(
            std::forward<Input>(input), std::forward<Handler>(handle_element));
    }
}

#endif
