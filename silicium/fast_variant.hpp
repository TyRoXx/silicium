#ifndef SILICIUM_FAST_VARIANT_HPP
#define SILICIUM_FAST_VARIANT_HPP

#include <silicium/variant.hpp>

namespace Si
{
#ifndef SILICIUM_NO_DEPRECATED
	//The name fast_variant is deprecated. You should use
	//variant instead.
#if SILICIUM_COMPILER_HAS_USING
	template <class ...T>
	using fast_variant = variant<T...>;
#endif

#endif
}

#endif
