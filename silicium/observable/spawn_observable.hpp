#ifndef SILICIUM_SPAWN_OBSERVABLE_HPP
#define SILICIUM_SPAWN_OBSERVABLE_HPP

#include <silicium/observable/extensible_observer.hpp>
#include <memory>

namespace Si
{
    namespace detail
    {
        template <class Next>
        struct observable_spawned
            : public std::enable_shared_from_this<observable_spawned<Next>>,
              observer<typename Next::element_type>
        {
            explicit observable_spawned(Next next)
                : m_next(std::move(next))
            {
            }

            void run()
            {
                fetch();
            }

        private:
            Next m_next;

            void fetch()
            {
                auto this_ = this->shared_from_this();
                m_next.async_get_one(make_extensible_observer(
                    static_cast<observer<typename Next::element_type> &>(*this),
                    this_));
            }

            void got_element(typename Next::element_type) SILICIUM_OVERRIDE
            {
                fetch();
            }

            void ended() SILICIUM_OVERRIDE
            {
            }
        };
    }

    template <class Observable>
    void spawn_observable(Observable &&observable)
    {
        auto spawned = std::make_shared<
            detail::observable_spawned<typename std::decay<Observable>::type>>(
            std::forward<Observable>(observable));
        spawned->run();
    }
}

#endif
