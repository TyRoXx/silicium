#ifndef SILICIUM_EMPTY_HPP
#define SILICIUM_EMPTY_HPP

#include <silicium/observable/observer.hpp>

namespace Si
{
	template <class Element>
	struct empty
	{
		typedef Element element_type;

		void async_get_one(observer<Element> &receiver)
		{
			return receiver.ended();
		}
	};
}

#endif
