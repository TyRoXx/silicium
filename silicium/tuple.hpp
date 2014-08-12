#ifndef SILICIUM_REACTIVE_TUPLE_HPP
#define SILICIUM_REACTIVE_TUPLE_HPP

#include <silicium/observer.hpp>
#include <silicium/config.hpp>
#include <silicium/detail/integer_sequence.hpp>
#include <silicium/override.hpp>
#include <bitset>

namespace Si
{
#ifdef _MSC_VER
#	define SILICIUM_RX_TUPLE_AVAILABLE 0
#else
#	define SILICIUM_RX_TUPLE_AVAILABLE 1
#endif

#if SILICIUM_RX_TUPLE_AVAILABLE
	template <class ...Parts>
	struct and_combinator
	{
		using element_type = std::tuple<typename Parts::element_type...>;
		typedef std::tuple<Parts...> parts_tuple;
		typedef element_type buffer_tuple;

		template <class PartsTuple>
		explicit and_combinator(PartsTuple &&parts)
			: parts(std::forward<PartsTuple>(parts))
		{
		}

#if !SILICIUM_COMPILER_GENERATES_MOVES
		and_combinator(and_combinator &&other)
			: parts(std::move(other.parts))
			, receiver(std::move(other.receiver))
			, buffer(std::move(other.buffer))
			, elements_received(std::move(other.elements_received))
			, observers(std::move(other.observers))
		{
		}

		and_combinator &operator = (and_combinator &&other)
		{
			parts = std::move(other.parts);
			receiver = std::move(other.receiver);
			buffer = std::move(other.buffer);
			elements_received = std::move(other.elements_received);
			observers = std::move(other.observers);
			return *this;
		}
#endif

		void async_get_one(observer<buffer_tuple> &receiver)
		{
			assert(!this->receiver);
			this->receiver = &receiver;
			this->elements_received.reset();
			return async_get_one_impl<0, Parts...>();
		}

		void cancel()
		{
			return cancel_impl<0, Parts...>();
		}

	private:

		template <class Element, std::size_t Index>
		struct tuple_observer : observer<Element>
		{
			and_combinator *combinator = nullptr;

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

		template <std::size_t ...I>
		struct make_observers<ranges::v3::integer_sequence<I...>>
		{
			typedef std::tuple<tuple_observer<typename Parts::element_type, I>...> type;
		};

		typedef typename make_observers<typename ranges::v3::make_integer_sequence<sizeof...(Parts)>::type>::type observers_type;

		parts_tuple parts;
		observer<buffer_tuple> *receiver = nullptr;
		buffer_tuple buffer;
		std::bitset<sizeof...(Parts)> elements_received;
		observers_type observers;

		template <std::size_t Index, class Head, class ...Tail>
		void async_get_one_impl()
		{
			auto &observer = std::get<Index>(observers);
			observer.combinator = this;
			auto &part = std::get<Index>(parts);
			part.async_get_one(observer);
			return async_get_one_impl<Index + 1, Tail...>();
		}

		template <std::size_t Index>
		void async_get_one_impl()
		{
		}

		template <std::size_t Index, class Head, class ...Tail>
		void cancel_impl()
		{
			assert(receiver);
			auto &part = std::get<Index>(parts);
			part.cancel();
			return cancel_impl<Index + 1, Tail...>();
		}

		template <std::size_t Index>
		void cancel_impl()
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

	template <class ...Parts>
	auto make_tuple(Parts &&...parts) -> and_combinator<typename std::decay<Parts>::type...>
	{
		return and_combinator<typename std::decay<Parts>::type...>(std::make_tuple(std::forward<Parts>(parts)...));
	}
#endif
}

#endif
