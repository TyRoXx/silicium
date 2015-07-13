#ifndef SILICIUM_FAST_VARIANT_HPP
#define SILICIUM_FAST_VARIANT_HPP

#include <silicium/variant.hpp>

namespace Si
{
	//The name fast_variant is deprecated. You should use
	//variant instead.
	template <class ...T>
	using fast_variant = variant<T...>;
}

#endif
