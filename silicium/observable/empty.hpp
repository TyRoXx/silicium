#ifndef SILICIUM_EMPTY_HPP
#define SILICIUM_EMPTY_HPP

#include <silicium/observable/observer.hpp>

namespace Si
{
	template <class Element>
	struct empty
	{
		typedef Element element_type;

		template <class Observer>
		void async_get_one(Observer &&receiver)
		{
			return std::forward<Observer>(receiver).ended();
		}
	};
}

#endif
