#ifndef SILICIUM_SILICIUM_CONFIG_HPP
#define SILICIUM_SILICIUM_CONFIG_HPP

#include <memory>
#include <boost/config.hpp>

#ifdef _MSC_VER
#	define SILICIUM_UNREACHABLE() __assume(false)
#else
#	define SILICIUM_UNREACHABLE() __builtin_unreachable()
#endif

#if (defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 408)) || defined(__clang__)
#	define SILICIUM_COMPILER_GENERATES_MOVES 1
#else
#	define SILICIUM_COMPILER_GENERATES_MOVES 0
#endif


#if defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 408) || defined(__clang__)
#	define SILICIUM_COMPILER_HAS_WORKING_NOEXCEPT 1
#else
#	define SILICIUM_COMPILER_HAS_WORKING_NOEXCEPT 0
#endif

#ifdef _MSC_VER
#	define SILICIUM_NORETURN __declspec(noreturn)
#else
//	GCC
#	define SILICIUM_NORETURN __attribute__ ((__noreturn__))
#endif

#if defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 408) || defined(__clang__)
#	define SILICIUM_COMPILER_HAS_RVALUE_THIS_QUALIFIER 1
#else
#	define SILICIUM_COMPILER_HAS_RVALUE_THIS_QUALIFIER 0
#endif

#if defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 408) || defined(__clang__)
#	define SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE 1
#else
#	define SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE 0
#endif

namespace Si
{
	struct nothing
	{
		BOOST_CONSTEXPR nothing() BOOST_NOEXCEPT
		{
		}
	};

	template <class T, class ...Args>
	auto make_unique(Args &&...args) -> std::unique_ptr<T>
	{
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}
}

#endif
