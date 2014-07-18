#ifndef SILICIUM_REACTIVE_CONFIG_HPP
#define SILICIUM_REACTIVE_CONFIG_HPP

#ifdef _MSC_VER
#define SILICIUM_BROKEN_VARIADIC_TEMPLATE_EXPANSION 1
#else
#define SILICIUM_BROKEN_VARIADIC_TEMPLATE_EXPANSION 0
#endif

namespace rx
{
	namespace detail
	{
		struct nothing
		{
		};
	}
}

#endif
