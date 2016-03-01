#ifndef SILICIUM_MAKE_UNIQUE_HPP
#define SILICIUM_MAKE_UNIQUE_HPP

#include <silicium/config.hpp>
#include <memory>

namespace Si
{
#if defined(_MSC_VER) && (_MSC_VER >= 1800)
	using std::make_unique;
#else

#if SILICIUM_COMPILER_HAS_VARIADIC_TEMPLATES
	template <class T, class... Args>
	std::unique_ptr<T> make_unique(Args &&... args)
	{
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}
#else  // SILICIUM_COMPILER_HAS_VARIADIC_TEMPLATES
	template <class T>
	std::unique_ptr<T> make_unique()
	{
		return std::unique_ptr<T>(new T());
	}

	template <class T, class A0>
	std::unique_ptr<T> make_unique(A0 &&a0)
	{
		return std::unique_ptr<T>(new T(std::forward<A0>(a0)));
	}

	template <class T, class A0, class A1>
	std::unique_ptr<T> make_unique(A0 &&a0, A1 &&a1)
	{
		return std::unique_ptr<T>(
		    new T(std::forward<A0>(a0), std::forward<A1>(a1)));
	}
#endif // SILICIUM_COMPILER_HAS_VARIADIC_TEMPLATES

#endif // defined(_MSC_VER) && (_MSC_VER >= 1800)
}

#endif
