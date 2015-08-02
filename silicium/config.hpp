#ifndef SILICIUM_SILICIUM_CONFIG_HPP
#define SILICIUM_SILICIUM_CONFIG_HPP

#include <memory>
#include <stdexcept>
#include <boost/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/version.hpp>
#include <boost/preprocessor/if.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_copy_assignable.hpp>
#include <boost/type_traits/is_copy_constructible.hpp>

#if defined(__GNUC__)
#	define SILICIUM_GCC ((__GNUC__ * 100) + __GNUC_MINOR__)
#else
#	define SILICIUM_GCC 0
#endif

#if defined(__GNUC__) && (__GNUC__ == 4) && (__GNUC_MINOR__ == 6)
#	define SILICIUM_GCC46 1
#else
#	define SILICIUM_GCC46 0
#endif

#ifdef _MSC_VER
#	define SILICIUM_VC2012 (_MSC_VER == 1700)
#	define SILICIUM_VC2013 (_MSC_VER == 1800)
#	define SILICIUM_VC2015 (_MSC_VER == 1900)
#else
#	define SILICIUM_VC2012 0
#	define SILICIUM_VC2013 0
#	define SILICIUM_VC2015 0
#endif

#ifdef _MSC_VER
#	define SILICIUM_COMPILER_CXX11 1
#	define SILICIUM_COMPILER_CXX14 1
#elif __cplusplus > 201103L
#	define SILICIUM_COMPILER_CXX11 1
#	define SILICIUM_COMPILER_CXX14 1
#elif __cplusplus >= 201103L
#	define SILICIUM_COMPILER_CXX11 1
#	define SILICIUM_COMPILER_CXX14 1 /* actually 0, assume 1 for now */
#else
#	define SILICIUM_COMPILER_CXX11 0
#	define SILICIUM_COMPILER_CXX14 0
#endif

#ifdef BOOST_NO_EXCEPTIONS
#	define SILICIUM_HAS_EXCEPTIONS 0
#else
#	define SILICIUM_HAS_EXCEPTIONS 1
#endif

#define SILICIUM_COMPILER_HAS_VARIADIC_TEMPLATES !SILICIUM_VC2012

#if defined(NDEBUG) || !SILICIUM_HAS_EXCEPTIONS
#	ifdef _MSC_VER
#		define SILICIUM_UNREACHABLE() __assume(false)
#	else
#		define SILICIUM_UNREACHABLE() __builtin_unreachable()
#	endif
#else
#	define SILICIUM_UNREACHABLE() throw ::std::logic_error("unreachable " __FILE__ ":" BOOST_STRINGIZE(__LINE__))
#endif

#if (defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 408)) || defined(__clang__) || (defined(_MSC_VER) && (_MSC_VER >= 1900))
#	define SILICIUM_COMPILER_GENERATES_MOVES 1
#else
#	define SILICIUM_COMPILER_GENERATES_MOVES 0
#endif

#if defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 408) || defined(__clang__) || (defined(_MSC_VER) && (_MSC_VER >= 1900))
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

#if (defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 408)) || defined(__clang__) || (defined(_MSC_VER) && (_MSC_VER >= 1900))
#	define SILICIUM_COMPILER_HAS_RVALUE_THIS_QUALIFIER 1
#else
#	define SILICIUM_COMPILER_HAS_RVALUE_THIS_QUALIFIER 0
#endif

#if (defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 408) && SILICIUM_COMPILER_CXX14) || defined(__clang__) || (defined(_MSC_VER) && (_MSC_VER >= 1900))
#	define SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE 1
#else
#	define SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE 0
#endif

#if defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 408) || defined(__clang__) || (defined(_MSC_VER) && (_MSC_VER >= 1900))
#	define SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE 1
#else
#	define SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE 0
#endif

#if SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE
#	define SILICIUM_CAPTURE_EXPRESSION(name, value) name = value
#else
#	define SILICIUM_CAPTURE_EXPRESSION(name, value) name
#endif

#if SILICIUM_COMPILER_HAS_VARIADIC_TEMPLATES && (__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 407) || defined(__clang__) || defined(_MSC_VER)
#	define SILICIUM_COMPILER_HAS_VARIADIC_PACK_EXPANSION 1
#else
#	define SILICIUM_COMPILER_HAS_VARIADIC_PACK_EXPANSION 0
#endif

#if defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 407) || defined(__clang__) || (defined(_MSC_VER) && (_MSC_VER >= 1800))
#	define SILICIUM_COMPILER_HAS_USING 1
#else
#	define SILICIUM_COMPILER_HAS_USING 0
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1900)
#	define SILICIUM_COMPILER_HAS_CXX11_UNION 0
#else
#	define SILICIUM_COMPILER_HAS_CXX11_UNION 1
#endif

#ifdef BOOST_DELETED_FUNCTION
#	define SILICIUM_DELETED_FUNCTION BOOST_DELETED_FUNCTION
#else
#	define SILICIUM_DELETED_FUNCTION(f) private: f;
#endif

#define SILICIUM_DISABLE_COPY(struct_name) \
	SILICIUM_DELETED_FUNCTION(struct_name(struct_name const &)) \
	SILICIUM_DELETED_FUNCTION(struct_name &operator = (struct_name const &))

#define SILICIUM_DEFAULT_NOEXCEPT_MOVE(struct_name) \
	struct_name(struct_name &&) BOOST_NOEXCEPT = default; \
	struct_name &operator = (struct_name &&) BOOST_NOEXCEPT = default;

#define SILICIUM_DEFAULT_MOVE(struct_name) \
	struct_name(struct_name &&) = default; \
	struct_name &operator = (struct_name &&) = default;

#define SILICIUM_DEFAULT_COPY(struct_name) \
	struct_name(struct_name const &) = default; \
	struct_name &operator = (struct_name const &) = default;

#ifdef _MSC_VER
#	define SILICIUM_DEPRECATED __declspec(deprecated)
#else
#	define SILICIUM_DEPRECATED __attribute__((deprecated))
#endif

#if defined(__GNUC__) && (__GNUC__ <= 4) && (__GNUC__ < 4 || __GNUC_MINOR__ <= 6)
#	define SILICIUM_OVERRIDE
#	define SILICIUM_FINAL
#else
#	define SILICIUM_OVERRIDE override
#	define SILICIUM_FINAL final
#endif

#ifdef _MSC_VER
#	define SILICIUM_USE_RESULT _Check_return_
#else
#	define SILICIUM_USE_RESULT __attribute__((warn_unused_result))
#endif

//TODO
#define SILICIUM_CXX14_CONSTEXPR

#define SILICIUM_IF(condition, value) BOOST_PP_IF(condition, value, BOOST_PP_EMPTY())
#define SILICIUM_IF_NOT(condition, value) BOOST_PP_IF(condition, BOOST_PP_EMPTY(), value)

#define SILICIUM_MOVE_IF_COMPILER_LACKS_RVALUE_QUALIFIERS(should_be_rvalue) BOOST_PP_IF(SILICIUM_COMPILER_HAS_RVALUE_THIS_QUALIFIER, (should_be_rvalue), std::move((should_be_rvalue)))

namespace Si
{
	struct nothing
	{
		BOOST_CONSTEXPR nothing() BOOST_NOEXCEPT
		{
		}
	};

	inline bool operator == (nothing const &, nothing const &)
	{
		return true;
	}

#if defined(_MSC_VER) && (_MSC_VER >= 1800)
	using std::make_unique;
#else

#if SILICIUM_COMPILER_HAS_VARIADIC_TEMPLATES
	template <class T, class ...Args>
	std::unique_ptr<T> make_unique(Args &&...args)
	{
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}
#else // SILICIUM_COMPILER_HAS_VARIADIC_TEMPLATES
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
		return std::unique_ptr<T>(new T(std::forward<A0>(a0), std::forward<A1>(a1)));
	}
#endif // SILICIUM_COMPILER_HAS_VARIADIC_TEMPLATES

#endif // defined(_MSC_VER) && (_MSC_VER >= 1800)

	template <class To, class From>
	To function_ptr_cast(From from)
	{
		//TODO: check that the cast makes sense
#ifdef __GNUC__
		//silence Warnung: ISO C++ forbids casting between pointer-to-function and pointer-to-object [enabled by default]
		__extension__
#endif
		To result = reinterpret_cast<To>(from);
		return result;
	}

#define SILICIUM_HAS_COPY_TRAITS !SILICIUM_VC2012

#if defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 407) || defined(__clang__)
#define SILICIUM_HAS_PROPER_COPY_TRAITS 1
}
#include <type_traits>
namespace Si
{
	BOOST_STATIC_ASSERT(SILICIUM_HAS_COPY_TRAITS);
	using std::is_copy_constructible;
	using std::is_copy_assignable;
#elif BOOST_VERSION >= 105500 //1.55
#define SILICIUM_HAS_PROPER_COPY_TRAITS 1
}
#endif

#include <boost/type_traits/is_copy_constructible.hpp>
#include <boost/type_traits/is_convertible.hpp>

#if SILICIUM_HAS_EXCEPTIONS
#	include <future>
#endif // defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 407) || defined(__clang__)

namespace Si
{
#if defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 407) || defined(__clang__) || defined(_MSC_VER)
}
#include <type_traits>
#include <boost/type_traits/is_nothrow_move_assignable.hpp>
namespace Si
{
	using std::is_default_constructible;
	using std::is_move_assignable;
	using std::is_move_constructible;
	using std::is_nothrow_default_constructible;
	using std::is_nothrow_move_constructible;
#if SILICIUM_VC2012
	template <class T>
	struct is_nothrow_move_assignable : std::integral_constant<bool, std::is_pod<T>::value || std::has_nothrow_copy_assign<T>::value>
	{
	};
#else // SILICIUM_VC2012
	using std::is_nothrow_move_assignable;
#endif // SILICIUM_VC2012

#if defined(_MSC_VER) && (_MSC_VER >= 1800)
	BOOST_STATIC_ASSERT(SILICIUM_HAS_COPY_TRAITS);
	using std::is_copy_assignable;
	using std::is_copy_constructible;
#endif // defined(_MSC_VER) && (_MSC_VER >= 1800)
	using std::is_nothrow_destructible;
#else // defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 407) || defined(__clang__) || defined(_MSC_VER)
	template <class T>
	struct is_default_constructible : std::conditional<
		std::is_reference<T>::value,
		std::false_type,
		std::is_constructible<T>
	>::type
	{
	};
	template <class T>
	struct is_nothrow_default_constructible : std::has_nothrow_default_constructor<T>
	{
	};
	template <class T>
	struct is_nothrow_move_constructible : std::is_nothrow_constructible<T, T>
	{
	};
	template <class T>
	struct is_nothrow_move_assignable : std::has_nothrow_copy_assign<T>
	{
	};
#if 0
	template <class T>
	struct is_nothrow_destructible : std::true_type
	{
	};
#endif
	template <class T>
	struct is_move_assignable : std::integral_constant<bool, __has_nothrow_assign(T)>
	{
	};
	template <class T>
	struct is_move_assignable<std::unique_ptr<T>> : std::true_type
	{
	};
	template <class T>
	struct is_move_constructible : std::integral_constant<bool, __has_nothrow_copy(T)>
	{
	};
	template <class T>
	struct is_move_constructible<std::unique_ptr<T>> : std::true_type
	{
	};
#endif // defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 407) || defined(__clang__) || defined(_MSC_VER)

	BOOST_STATIC_ASSERT(is_default_constructible<int>::value);
	BOOST_STATIC_ASSERT(is_nothrow_default_constructible<int>::value);
	BOOST_STATIC_ASSERT(is_nothrow_move_constructible<int>::value);
	BOOST_STATIC_ASSERT(is_nothrow_move_assignable<int>::value);
	BOOST_STATIC_ASSERT(is_nothrow_destructible<int>::value);
	BOOST_STATIC_ASSERT(is_move_assignable<int>::value);
	BOOST_STATIC_ASSERT(is_move_constructible<int>::value);
#if SILICIUM_HAS_COPY_TRAITS
	BOOST_STATIC_ASSERT(is_copy_constructible<int>::value);
	BOOST_STATIC_ASSERT(is_copy_assignable<int>::value);
#endif

	BOOST_STATIC_ASSERT(is_default_constructible<std::unique_ptr<int>>::value);
#if SILICIUM_COMPILER_HAS_WORKING_NOEXCEPT
	BOOST_STATIC_ASSERT(is_nothrow_default_constructible<std::unique_ptr<int>>::value);
	BOOST_STATIC_ASSERT(is_nothrow_move_constructible<std::unique_ptr<int>>::value);
	BOOST_STATIC_ASSERT(is_nothrow_move_assignable<std::unique_ptr<int>>::value);
	BOOST_STATIC_ASSERT(is_nothrow_destructible<std::unique_ptr<int>>::value);
#endif
	BOOST_STATIC_ASSERT(is_move_assignable<std::unique_ptr<int>>::value);
	BOOST_STATIC_ASSERT(is_move_constructible<std::unique_ptr<int>>::value);
#if SILICIUM_HAS_COPY_TRAITS
	BOOST_STATIC_ASSERT(!is_copy_constructible<std::unique_ptr<int>>::value);
	BOOST_STATIC_ASSERT(!is_copy_assignable<std::unique_ptr<int>>::value);
#endif

#if BOOST_VERSION <= 105400
#	if defined(_MSC_VER) && (_MSC_VER < 1900)
		typedef std::size_t uintptr_t;
#	else
		typedef std::uintptr_t uintptr_t;
#	endif
#else
	typedef boost::uintptr_t uintptr_t;
#endif
}

#endif
