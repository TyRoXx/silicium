#ifndef SILICIUM_REACTIVE_BRIDGE_HPP
#define SILICIUM_REACTIVE_BRIDGE_HPP

#include <reactive/observer.hpp>
#include <reactive/observable.hpp>

namespace rx
{
	template <class T, class U>
	T exchange(T &dest, U &&source)
	{
		auto old = dest;
		dest = std::forward<U>(source);
		return old;
	}

	template <class Element>
	struct bridge : observable<Element>, observer<Element>
	{
		typedef Element element_type;

		bool is_waiting() const
		{
			return receiver != nullptr;
		}

		virtual void got_element(element_type value) SILICIUM_OVERRIDE
		{
			assert(receiver);
			exchange(receiver, nullptr)->got_element(std::move(value));
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			assert(receiver);
			exchange(receiver, nullptr)->ended();
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(!this->receiver);
			this->receiver = &receiver;
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			assert(receiver);
			receiver = nullptr;
		}

	private:

		observer<element_type> *receiver = nullptr;
	};
}

#endif
