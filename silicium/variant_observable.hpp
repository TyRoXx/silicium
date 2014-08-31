#ifndef SILICIUM_REACTIVE_VARIANT_OBSERVABLE_HPP
#define SILICIUM_REACTIVE_VARIANT_OBSERVABLE_HPP

#include <silicium/observer.hpp>
#include <silicium/exchange.hpp>
#include <silicium/config.hpp>
#include <silicium/detail/integer_sequence.hpp>
#include <silicium/fast_variant.hpp>
#include <silicium/override.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace Si
{
#define SILICIUM_RX_VARIANT_AVAILABLE 1

#if SILICIUM_RX_VARIANT_AVAILABLE
	template <template <class ...T> class variant, class Lockable, class ...Parts>
	struct variant_observable
	{
		typedef variant<typename Parts::element_type...> element_type;

		template <class ...P>
		explicit variant_observable(P &&...parts)
			: parts(std::forward<P>(parts)...)
			, mutex(new Lockable)
		{
		}

#if SILICIUM_COMPILER_GENERATES_MOVES
		variant_observable(variant_observable &&other) = default;
		variant_observable &operator = (variant_observable &&other) = default;
#else
		variant_observable(variant_observable &&other)
			: parts(std::move(other.parts))
			, observers(std::move(other.observers))
			, receiver_(other.receiver_)
			, mutex(std::move(other.mutex))
		{
		}

		variant_observable &operator = (variant_observable &&other)
		{
			parts = std::move(other.parts);
			observers = std::move(other.observers);
			receiver_ = other.receiver_;
			mutex = std::move(other.mutex);
			return *this;
		}
#endif

		void async_get_one(Si::observer<element_type> &receiver)
		{
			boost::unique_lock<Lockable> lock(*mutex);
			assert(!receiver_);
			receiver_ = &receiver;
			return async_get_one_impl<0, Parts...>();
		}

	private:

		template <class Element, std::size_t Index>
		struct tuple_observer : observer<Element>
		{
			variant_observable *combinator = nullptr;
			boost::optional<Element> cached;

			virtual void got_element(Element value) SILICIUM_OVERRIDE
			{
				assert(!cached);
				boost::unique_lock<Lockable> lock(*combinator->mutex);
				auto * const receiver = Si::exchange(Si::exchange(combinator, nullptr)->receiver_, nullptr);
				if (!receiver)
				{
					cached = std::move(value);
					return;
				}
				lock.unlock();
				receiver->got_element(element_type{std::move(value)});
			}

			virtual void ended() SILICIUM_OVERRIDE
			{
				boost::unique_lock<Lockable> lock(*combinator->mutex);
				auto * const receiver = Si::exchange(Si::exchange(combinator, nullptr)->receiver_, nullptr);
				if (!receiver)
				{
					return;
				}
				lock.unlock();
				receiver->ended();
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

		std::tuple<Parts...> parts;
		observers_type observers;
		Si::observer<element_type> *receiver_ = nullptr;
		std::unique_ptr<Lockable> mutex;

		template <std::size_t Index, class Head, class ...Tail>
		void async_get_one_impl()
		{
			auto &observer = std::get<Index>(observers);
			if (!observer.combinator)
			{
				if (observer.cached)
				{
					auto value = std::move(*observer.cached);
					Si::exchange(receiver_, nullptr)->got_element(std::move(value));
					return;
				}
				observer.combinator = this;
				auto &part = std::get<Index>(parts);
				part.async_get_one(observer);
			}
			return async_get_one_impl<Index + 1, Tail...>();
		}

		template <std::size_t Index>
		void async_get_one_impl()
		{
		}

		BOOST_DELETED_FUNCTION(variant_observable(variant_observable const &))
		BOOST_DELETED_FUNCTION(variant_observable &operator = (variant_observable const &))
	};

	template <class Lockable = boost::recursive_mutex, class ...Parts>
	auto make_variant(Parts &&...parts)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> variant_observable<Si::fast_variant, Lockable, typename std::decay<Parts>::type...>
#endif
	{
		return variant_observable<Si::fast_variant, Lockable, typename std::decay<Parts>::type...>(std::forward<Parts>(parts)...);
	}

	template <template <class ...T> class variant, class Lockable = boost::recursive_mutex, class ...Parts>
	auto make_variant(Parts &&...parts)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> variant_observable<variant, Lockable, typename std::decay<Parts>::type...>
#endif
	{
		return variant_observable<variant, Lockable, typename std::decay<Parts>::type...>(std::forward<Parts>(parts)...);
	}
#endif
}

#endif
