#ifndef SILICIUM_SILICIUM_CONFIG_HPP
#define SILICIUM_SILICIUM_CONFIG_HPP

#include <memory>
#include <stdexcept>
#include <boost/config.hpp>
#include <boost/version.hpp>

#ifdef _MSC_VER
//avoid useless warning C4127 (conditional expression is constant)
#	define SILICIUM_FALSE (!"")
#else
#	define SILICIUM_FALSE false
#endif

#ifdef NDEBUG
#	ifdef _MSC_VER
#		define SILICIUM_UNREACHABLE() __assume(false)
#	else
#		define SILICIUM_UNREACHABLE() __builtin_unreachable()
#	endif
#else
#	define SILICIUM_UNREACHABLE() do { throw ::std::logic_error("unreachable " __FILE__ ":" BOOST_STRINGIZE(__LINE__)); } while(SILICIUM_FALSE)
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

#if defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 408) || defined(__clang__)
#	define SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE 1
#else
#	define SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE 0
#endif

#if defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 407) || defined(__clang__) || defined(_MSC_VER)
#	define SILICIUM_COMPILER_HAS_VARIADIC_PACK_EXPANSION 1
#else
#	define SILICIUM_COMPILER_HAS_VARIADIC_PACK_EXPANSION 0
#endif

#if defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 407) || defined(__clang__) || defined(_MSC_VER)
#	define SILICIUM_COMPILER_HAS_USING 1
#else
#	define SILICIUM_COMPILER_HAS_USING 0
#endif

#ifdef BOOST_DELETED_FUNCTION
#	define SILICIUM_DELETED_FUNCTION BOOST_DELETED_FUNCTION
#else
#	define SILICIUM_DELETED_FUNCTION(f) private: f;
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

#if defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 407) || defined(__clang__) || defined(_MSC_VER)
#define SILICIUM_HAS_PROPER_COPY_TRAITS 1
}
#include <type_traits>
namespace Si
{
	using std::is_copy_constructible;
	using std::is_copy_assignable;
	using std::is_default_constructible;
	using std::is_move_assignable;
#elif BOOST_VERSION >= 105500 //1.55
#define SILICIUM_HAS_PROPER_COPY_TRAITS 1
}
#include <boost/type_traits/is_copy_constructible.hpp>
#include <boost/type_traits/is_default_constructible.hpp>
#include <boost/type_traits/is_move_assignable.hpp>
namespace Si
{
	using boost::is_copy_constructible;
	using boost::is_copy_assignable;
	using boost::is_default_constructible;
	using boost::is_move_assignable;
#else
#define SILICIUM_HAS_PROPER_COPY_TRAITS 0
	template <class T>
	struct is_copy_constructible : std::true_type
	{
	};
	template <class T>
	struct is_copy_assignable : std::true_type
	{
	};
	template <class T>
	struct is_default_constructible : std::false_type
	{
	};
	template <class T>
	struct is_move_assignable : std::false_type
	{
	};
#endif
}

#endif
