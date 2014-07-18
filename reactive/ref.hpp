#ifndef SILICIUM_REACTIVE_REF_HPP
#define SILICIUM_REACTIVE_REF_HPP

#include <reactive/ptr_observable.hpp>

namespace rx
{
	template <class Element>
	auto ref(rx::observable<Element> &identity) -> rx::ptr_observable<Element, rx::observable<Element> *>
	{
		return rx::ptr_observable<Element, rx::observable<Element> *>(&identity);
	}
}

#endif
