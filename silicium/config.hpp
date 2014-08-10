#ifndef SILICIUM_SILICIUM_CONFIG_HPP
#define SILICIUM_SILICIUM_CONFIG_HPP

#include <memory>

#ifdef _MSC_VER
#	define SILICIUM_UNREACHABLE() __assume(false)
#else
#	define SILICIUM_UNREACHABLE() __builtin_unreachable()
#endif

namespace Si
{
	struct nothing
	{
	};

	template <class T, class ...Args>
	auto make_unique(Args &&...args) -> std::unique_ptr<T>
	{
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}
}

namespace rx
{
	using Si::nothing;
	using Si::make_unique;
}

#endif
