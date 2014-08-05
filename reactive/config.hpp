#ifndef SILICIUM_REACTIVE_CONFIG_HPP
#define SILICIUM_REACTIVE_CONFIG_HPP

#include <memory>

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
	auto to_unique(T &&t) -> std::unique_ptr<typename std::decay<T>::type>
	{
		typedef typename std::decay<T>::type decayed_T;
		return std::unique_ptr<decayed_T>(new decayed_T(std::forward<T>(t)));
	}
}

#endif
