#ifndef SILICIUM_TYPE_TRAITS_HPP
#define SILICIUM_TYPE_TRAITS_HPP

#include <silicium/config.hpp>
#include <boost/static_assert.hpp>
#if BOOST_VERSION >= 105700
#include <boost/type_traits/is_copy_assignable.hpp>
#endif
#if BOOST_VERSION >= 105500
#include <boost/type_traits/is_copy_constructible.hpp>
#endif
#include <boost/type_traits/has_trivial_constructor.hpp>
#if BOOST_VERSION >= 105400
#include <boost/type_traits/is_nothrow_move_constructible.hpp>
#include <boost/type_traits/is_nothrow_move_assignable.hpp>
#endif
#include <boost/type_traits/has_nothrow_destructor.hpp>

#define SILICIUM_HAS_COPY_TRAITS ((!SILICIUM_VC || SILICIUM_VC2013_OR_LATER) && !SILICIUM_GCC46)

#if defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 407) || defined(__clang__)
#define SILICIUM_HAS_PROPER_COPY_TRAITS 1
#include <type_traits>
namespace Si
{
	BOOST_STATIC_ASSERT(SILICIUM_HAS_COPY_TRAITS);
	using std::is_copy_constructible;
	using std::is_copy_assignable;
}
#elif BOOST_VERSION >= 105700
#define SILICIUM_HAS_PROPER_COPY_TRAITS 1
#endif

#include <boost/type_traits/is_convertible.hpp>

namespace Si
{
#if defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 407) || defined(__clang__) || SILICIUM_VC2012_OR_LATER
}
#include <type_traits>
namespace Si
{
	using std::is_default_constructible;
	using std::is_move_assignable;
	using std::is_move_constructible;
	using std::is_nothrow_default_constructible;
	using std::is_nothrow_move_constructible;
#if SILICIUM_VC2012
	template <class T>
	struct is_nothrow_move_assignable
	    : std::integral_constant<bool, std::is_pod<T>::value || std::has_nothrow_copy_assign<T>::value>
	{
	};
#else  // SILICIUM_VC2012
	using std::is_nothrow_move_assignable;
#endif // SILICIUM_VC2012

#if defined(_MSC_VER) && (_MSC_VER >= 1800)
	BOOST_STATIC_ASSERT(SILICIUM_HAS_COPY_TRAITS);
#if _MSC_VER >= 1900
	using std::is_copy_assignable;
	using std::is_copy_constructible;
#else

#if BOOST_VERSION >= 105700
	template <class T>
	struct is_copy_assignable : boost::is_copy_assignable<T>
	{
	};
	template <class T, class D>
	struct is_copy_assignable<std::unique_ptr<T, D>> : std::false_type
	{
	};
#else
	template <class T>
	struct is_copy_assignable : std::true_type
	{
	};
#endif
	template <class T>
	struct is_copy_constructible : boost::is_copy_constructible<T>
	{
	};
	template <class T, class D>
	struct is_copy_constructible<std::unique_ptr<T, D>> : std::false_type
	{
	};
#endif

#endif // defined(_MSC_VER) && (_MSC_VER >= 1800)

#if defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) == 407)
	template <class T>
	struct is_nothrow_destructible : std::true_type
	{
	};
#else
	using std::is_nothrow_destructible;
#endif

#else // defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 407) || defined(__clang__) || defined(_MSC_VER)
	template <class T>
	struct is_default_constructible : std::conditional<std::is_reference<T>::value, std::false_type,
#if !defined(_MSC_VER) || SILICIUM_VC2012_OR_LATER
	                                                   std::is_constructible<T>
#else
	                                                   boost::has_trivial_constructor<T>
#endif
	                                                   >::type
	{
	};
	template <class T>
	struct is_nothrow_default_constructible : std::has_nothrow_default_constructor<T>
	{
	};

#if !defined(_MSC_VER) || SILICIUM_VC2012_OR_LATER
	template <class T>
	struct is_nothrow_move_constructible : std::is_nothrow_constructible<T, T>
	{
	};
#else
	using boost::is_nothrow_move_constructible;
#endif

#if !defined(_MSC_VER) || SILICIUM_VC2012_OR_LATER
	template <class T>
	struct is_nothrow_move_assignable : std::has_nothrow_copy_assign<T>
	{
	};
#else
	using boost::is_nothrow_move_assignable;
#endif

#if (!defined(_MSC_VER) || SILICIUM_VC2012_OR_LATER) && !SILICIUM_GCC46
	using std::is_nothrow_destructible;
#else
	template <class T>
	struct is_nothrow_destructible : boost::has_nothrow_destructor<T>
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
#endif
}

#endif
