#ifndef SILICIUM_OBSERVABLE_HPP
#define SILICIUM_OBSERVABLE_HPP

#include <silicium/observable/observer.hpp>
#include <silicium/trait.hpp>

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

	template <class Element, class Observer>
	SILICIUM_TRAIT_WITH_TYPEDEFS(
		Observable,
		typedef Element element_type;
		,
		((async_get_one, (1, (Observer)), void))
	)
}

#endif
