#ifndef SILICIUM_REACTIVE_REF_HPP
#define SILICIUM_REACTIVE_REF_HPP

#include <silicium/ptr_observable.hpp>

namespace Si
{
	template <class Element>
	using reference = ptr_observable<Element, observable<Element> *>;

	template <class Element>
	auto ref(observable<Element> &identity) -> reference<Element>
	{
		return reference<Element>(&identity);
	}
}

#endif
