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
	struct nothing
	{
	};

	template <class T, class ...Args>
	auto make_unique(Args &&...args) -> std::unique_ptr<T>
	{
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}

	template <class T>
	auto to_unique(T &&t)
	{
		typedef typename std::decay<T>::type decayed_T;
		return std::unique_ptr<decayed_T>(new decayed_T(std::forward<T>(t)));
	}
}

#endif
