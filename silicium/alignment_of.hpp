#ifndef SILICIUM_ALIGNMENT_OF_HPP
#define SILICIUM_ALIGNMENT_OF_HPP

#include <silicium/config.hpp>

namespace Si
{
	template <class T>
	struct alignment_of : std::integral_constant<std::size_t,
#ifdef _MSC_VER
	                                             __alignof(T)
#else
	                                             alignof(T)
#endif
	                                             >
	{
	};
}

#endif
