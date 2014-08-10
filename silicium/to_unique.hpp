#ifndef SILICIUM_TO_UNIQUE_HPP
#define SILICIUM_TO_UNIQUE_HPP

#include <memory>

namespace Si
{
	template <class T>
	auto to_unique(T &&t) -> std::unique_ptr<typename std::decay<T>::type>
	{
		typedef typename std::decay<T>::type decayed_T;
		return std::unique_ptr<decayed_T>(new decayed_T(std::forward<T>(t)));
	}
}

namespace Si
{
	using Si::to_unique;
}

#endif
