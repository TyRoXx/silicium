#ifndef SILICIUM_REACTIVE_OBSERVABLE_HPP
#define SILICIUM_REACTIVE_OBSERVABLE_HPP

#include <reactive/observer.hpp>

namespace rx
{
	template <class E>
	struct observable
	{
		typedef E element_type;

		virtual ~observable()
		{
		}

		virtual void async_get_one(observer<element_type> &receiver) = 0;
		virtual void cancel() = 0;
	};
}

#endif
