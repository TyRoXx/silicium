#ifndef SILICIUM_REACTIVE_CONFIG_HPP
#define SILICIUM_REACTIVE_CONFIG_HPP

#include <memory>

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

	using nothing = detail::nothing;

	template <class T, class ...Args>
	auto make_unique(Args &&...args)
	{
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}

}

#endif
