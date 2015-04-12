#ifndef SILICIUM_OBSERVABLE_HPP
#define SILICIUM_OBSERVABLE_HPP

#include <silicium/observable/observer.hpp>

namespace Si
{
	template <class Element, class Observer>
	struct observable
	{
		typedef Element element_type;

		virtual ~observable()
		{
		}

		virtual void async_get_one(Observer receiver) = 0;
	};
}

#endif
