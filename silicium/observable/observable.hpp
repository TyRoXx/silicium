#ifndef SILICIUM_OBSERVABLE_HPP
#define SILICIUM_OBSERVABLE_HPP

#include <silicium/trait.hpp>

namespace Si
{
	template <class Element, class Observer>
	SILICIUM_TRAIT_WITH_TYPEDEFS(
		Observable,
		typedef Element element_type;
		,
		((async_get_one, (1, (Observer)), void))
	)

	template <class Element, class Observer>
	using observable = typename Observable<Element, Observer>::interface;
}

#endif
