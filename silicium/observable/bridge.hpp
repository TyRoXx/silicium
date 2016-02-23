#ifndef SILICIUM_REACTIVE_BRIDGE_HPP
#define SILICIUM_REACTIVE_BRIDGE_HPP

#include <silicium/observable/observer.hpp>
#include <silicium/observable/observable.hpp>
#include <silicium/exchange.hpp>
#include <silicium/config.hpp>
#include <cassert>

namespace Si
{
	template <class Element>
	struct bridge
	    : Observable<Element, ptr_observer<observer<Element>>>::interface,
	      observer<Element>
	{
		typedef Element element_type;

		bridge()
		    : receiver_(nullptr)
		{
		}

		bool is_waiting() const
		{
			return receiver_ != nullptr;
		}

		virtual void got_element(element_type value) SILICIUM_OVERRIDE
		{
			assert(receiver_);
			Si::exchange(receiver_, nullptr)->got_element(std::move(value));
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			assert(receiver_);
			Si::exchange(receiver_, nullptr)->ended();
		}

		virtual void async_get_one(
		    ptr_observer<observer<element_type>> receiver) SILICIUM_OVERRIDE
		{
			assert(!this->receiver_);
			this->receiver_ = receiver.get();
		}

	private:
		observer<element_type> *receiver_;
	};
}

#endif
