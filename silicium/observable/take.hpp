#ifndef SILICIUM_TAKE_OBSERVABLE_HPP
#define SILICIUM_TAKE_OBSERVABLE_HPP

#include <silicium/config.hpp>
#include <silicium/observable/observer.hpp>

namespace Si
{
    template <class Input, class Counter>
    struct take_observable
    {
        typedef typename Input::element_type element_type;

        take_observable()
        {
        }

        explicit take_observable(Input input, Counter remaining_count)
            : input(std::move(input))
            , remaining_count(std::move(remaining_count))
        {
        }

        template <class Observer>
        void async_get_one(Observer &&receiver)
        {
            if (remaining_count)
            {
                --remaining_count;
                return input.async_get_one(std::forward<Observer>(receiver));
            }
            else
            {
                std::forward<Observer>(receiver).ended();
            }
        }

    private:
        Input input;
        Counter remaining_count;
    };

    template <class Input, class Counter>
    auto take(Input &&input, Counter &&count)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
        -> take_observable<typename std::decay<Input>::type,
                           typename std::decay<Counter>::type>
#endif
    {
        return take_observable<typename std::decay<Input>::type,
                               typename std::decay<Counter>::type>(
            std::forward<Input>(input), std::forward<Counter>(count));
    }
}

#endif
