#ifndef SILICIUM_TO_UNIQUE_HPP
#define SILICIUM_TO_UNIQUE_HPP

#include <silicium/config.hpp>
#include <memory>

namespace Si
{
	template <class T>
	auto to_unique(T &&t)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> std::unique_ptr<typename std::decay<T>::type>
#endif
	{
		typedef typename std::decay<T>::type decayed_T;
		return std::unique_ptr<decayed_T>(new decayed_T(std::forward<T>(t)));
	}

	template <class Pointee, class T>
	auto to_unique(T &&t)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> std::unique_ptr<Pointee>
#endif
	{
		typedef typename std::decay<T>::type decayed_T;
		return std::unique_ptr<Pointee>(new decayed_T(std::forward<T>(t)));
	}
}

#endif
