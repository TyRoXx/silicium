#ifndef SILICIUM_REACTIVE_BRIDGE_HPP
#define SILICIUM_REACTIVE_BRIDGE_HPP

#include <silicium/observer.hpp>
#include <silicium/observable.hpp>
#include <silicium/exchange.hpp>
#include <silicium/override.hpp>
#include <cassert>

namespace Si
{
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
			Si::exchange(receiver, nullptr)->got_element(std::move(value));
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			assert(receiver);
			Si::exchange(receiver, nullptr)->ended();
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(!this->receiver);
			this->receiver = &receiver;
		}

	private:

		observer<element_type> *receiver = nullptr;
	};
}

#endif
