#ifndef SILICIUM_REACTIVE_TUPLE_HPP
#define SILICIUM_REACTIVE_TUPLE_HPP

#include <silicium/observable/observer.hpp>
#include <silicium/config.hpp>
#include <bitset>
#include <tuple>

#if (defined(_MSC_VER) && (MSC_VER < 1900)) || !SILICIUM_COMPILER_HAS_USING || \
    SILICIUM_GCC47 /*GCC 4.7 crashes when using tuple*/
#define SILICIUM_RX_TUPLE_AVAILABLE 0
#else
#define SILICIUM_RX_TUPLE_AVAILABLE 1
#endif

#if SILICIUM_RX_TUPLE_AVAILABLE
#include <silicium/detail/integer_sequence.hpp>
#endif

namespace Si
{
#if SILICIUM_RX_TUPLE_AVAILABLE
    template <class... Parts>
    struct tuple_observable
    {
        typedef std::tuple<typename Parts::element_type...> element_type;
        typedef std::tuple<Parts...> parts_tuple;
        typedef element_type buffer_tuple;

        template <class PartsTuple>
        explicit tuple_observable(PartsTuple &&parts)
            : parts(std::forward<PartsTuple>(parts))
        {
        }

#if !SILICIUM_COMPILER_GENERATES_MOVES
        tuple_observable(tuple_observable &&other)
            : parts(std::move(other.parts))
            , receiver(std::move(other.receiver))
            , buffer(std::move(other.buffer))
            , elements_received(std::move(other.elements_received))
            , observers(std::move(other.observers))
        {
        }

        tuple_observable(tuple_observable const &other)
            : parts(other.parts)
            , receiver(other.receiver)
            , buffer(other.buffer)
            , elements_received(other.elements_received)
            , observers(other.observers)
        {
        }

        tuple_observable &operator=(tuple_observable &&other)
        {
            parts = std::move(other.parts);
            receiver = std::move(other.receiver);
            buffer = std::move(other.buffer);
            elements_received = std::move(other.elements_received);
            observers = std::move(other.observers);
            return *this;
        }

        tuple_observable &operator=(tuple_observable const &other)
        {
            parts = other.parts;
            receiver = other.receiver;
            buffer = other.buffer;
            elements_received = other.elements_received;
            observers = other.observers;
            return *this;
        }
#endif

        void async_get_one(ptr_observer<observer<buffer_tuple>> receiver)
        {
            assert(!this->receiver);
            this->receiver = receiver.get();
            this->elements_received.reset();
            return async_get_one_impl<0, Parts...>();
        }

    private:
        template <class Element, std::size_t Index>
        struct tuple_observer : observer<Element>
        {
            tuple_observable *combinator = nullptr;

            virtual void got_element(Element value) SILICIUM_OVERRIDE
            {
                std::get<Index>(combinator->buffer) = std::move(value);
                combinator->elements_received.set(Index);
                combinator->check_received();
            }

            virtual void ended() SILICIUM_OVERRIDE
            {
                auto *receiver_copy = combinator->receiver;
                combinator->receiver = nullptr;
                receiver_copy->ended();
            }
        };

        template <class Indices>
        struct make_observers;

        template <std::size_t... I>
        struct make_observers<ranges::v3::integer_sequence<I...>>
        {
            typedef std::tuple<
                tuple_observer<typename Parts::element_type, I>...> type;
        };

        typedef
            typename make_observers<typename ranges::v3::make_integer_sequence<
                sizeof...(Parts)>::type>::type observers_type;

        parts_tuple parts;
        observer<buffer_tuple> *receiver = nullptr;
        buffer_tuple buffer;
        std::bitset<sizeof...(Parts)> elements_received;
        observers_type observers;

        template <std::size_t Index, class Head, class... Tail>
        void async_get_one_impl()
        {
            auto &observer = std::get<Index>(observers);
            observer.combinator = this;
            auto &part = std::get<Index>(parts);
            part.async_get_one(observe_by_ref(observer));
            return async_get_one_impl<Index + 1, Tail...>();
        }

        template <std::size_t Index>
        void async_get_one_impl()
        {
        }

        void check_received()
        {
            if (!elements_received.all())
            {
                return;
            }

            auto *receiver_copy = receiver;
            receiver = nullptr;
            receiver_copy->got_element(std::move(buffer));
        }
    };

    template <class... Parts>
    auto make_tuple(Parts &&... parts)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
        -> tuple_observable<typename std::decay<Parts>::type...>
#endif
    {
        return tuple_observable<typename std::decay<Parts>::type...>(
            std::make_tuple(std::forward<Parts>(parts)...));
    }
#endif
}

#endif
