#ifndef SILICIUM_REACTIVE_OBSERVABLE_HPP
#define SILICIUM_REACTIVE_OBSERVABLE_HPP

#include <silicium/observer.hpp>

namespace Si
{
	template <class E>
	struct observable
	{
		typedef E element_type;

		virtual ~observable()
		{
		}

		virtual void async_get_one(observer<element_type> &receiver) = 0;
	};
}

#endif
