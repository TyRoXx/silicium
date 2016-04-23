#ifndef SILICIUM_ASIO_POSTING_OBSERVABLE_HPP
#define SILICIUM_ASIO_POSTING_OBSERVABLE_HPP

#include <silicium/exchange.hpp>
#include <silicium/observable/function_observer.hpp>
#include <silicium/to_shared.hpp>
#include <silicium/make_unique.hpp>
#include <algorithm>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>

namespace Si
{
    namespace asio
    {
        inline boost::asio::io_service &
        get_io_service(boost::asio::io_service &io)
        {
            return io;
        }

        inline boost::asio::io_service &
        get_io_service(boost::asio::io_service::strand &strand)
        {
            return strand.get_io_service();
        }

        template <class Next, class Dispatcher = boost::asio::io_service>
        struct posting_observable
        {
            typedef typename Next::element_type element_type;

            posting_observable()
                : m_io(nullptr)
            {
            }

            explicit posting_observable(Dispatcher &io, Next next)
                : m_io(&io)
                , m_next(std::move(next))
            {
            }

            template <class Observer>
            void async_get_one(Observer &&observer_)
            {
                auto keep_io_running =
#if SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE
                    Si::make_unique
#else
                    std::make_shared
#endif
                    <boost::asio::io_service::work>(get_io_service(*m_io));

                // VC++ 2013 bug: this cannot be captured by the following
                // lambda for some reason (fails with
                // nonsense compiler error about a generated posting_observable
                // copy operator)
                Dispatcher *io = this->m_io;
                m_next.async_get_one(make_function_observer(
                    [
                      io,
                      SILICIUM_CAPTURE_EXPRESSION(
                          keep_io_running, std::move(keep_io_running)),
                      SILICIUM_CAPTURE_EXPRESSION(
                          observer_, std::forward<Observer>(observer_))
                    ](Si::optional<element_type> element) mutable
                    {
                        if (element)
                        {
#if SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE
                            io->post(
                                // The additional indirection makes the function
                                // object copyable even
                                // if element_type is not. post() would not take
                                // a
                                // noncopyable function.
                                // TODO: do this only when necessary
                                function<void()>(
                                    [
                                      SILICIUM_CAPTURE_EXPRESSION(
                                          element, std::move(element)),
                                      SILICIUM_CAPTURE_EXPRESSION(
                                          observer_,
                                          std::forward<Observer>(observer_))
                                    ]() mutable
                                    {
                                        std::forward<Observer>(observer_)
                                            .got_element(std::move(*element));
                                    }));
#else
                            auto copyable_element =
                                to_shared(std::move(*element));
                            io->post([copyable_element, observer_]() mutable
                                     {
                                         std::forward<Observer>(observer_)
                                             .got_element(
                                                 std::move(*copyable_element));
                                     });
#endif
                        }
                        else
                        {
                            io->post([SILICIUM_CAPTURE_EXPRESSION(
                                observer_,
                                std::forward<Observer>(observer_))]() mutable
                                     {
                                         std::forward<Observer>(observer_)
                                             .ended();
                                     });
                        }
                    }));
            }

#if SILICIUM_COMPILER_GENERATES_MOVES
            posting_observable(posting_observable &&) = default;
            posting_observable &operator=(posting_observable &&) = default;
#else
            posting_observable(posting_observable &&other)
                : m_io(std::move(other.m_io))
                , m_next(std::move(other.m_next))
            {
            }

            posting_observable &operator=(posting_observable &&other)
            {
                m_io = std::move(other.m_io);
                m_next = std::move(other.m_next);
                return *this;
            }
#endif

            Next &get_input()
            {
                return m_next;
            }

        private:
            Dispatcher *m_io;
            Next m_next;

            SILICIUM_DELETED_FUNCTION(
                posting_observable(posting_observable const &))
            SILICIUM_DELETED_FUNCTION(
                posting_observable &operator=(posting_observable const &))
        };

        template <class Next>
        auto make_posting_observable(boost::asio::io_service &io, Next &&next)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
            -> posting_observable<typename std::decay<Next>::type>
#endif
        {
            return posting_observable<typename std::decay<Next>::type>(
                io, std::forward<Next>(next));
        }
    }
}

#endif
