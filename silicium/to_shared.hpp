#ifndef SILICIUM_TO_SHARED_HPP
#define SILICIUM_TO_SHARED_HPP

#include <silicium/config.hpp>
#include <memory>

namespace Si
{
	template <class T>
	auto to_shared(T &&t)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> std::shared_ptr<typename std::decay<T>::type>
#endif
	{
		typedef typename std::decay<T>::type decayed_T;
		return std::make_shared<decayed_T>(std::forward<T>(t));
	}
}

#endif
