#ifndef SILICIUM_REACTIVE_REF_HPP
#define SILICIUM_REACTIVE_REF_HPP

#include <silicium/observable/ptr_observable.hpp>

namespace Si
{
	template <class Observable, class Element = typename Observable::element_type>
	auto ref(Observable &identity)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> ptr_observable<Element, Observable *>
#endif
	{
		return ptr_observable<Element, Observable *>(&identity);
	}
}

#endif
