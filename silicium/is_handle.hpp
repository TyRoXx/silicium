#ifndef SILICIUM_IS_HANDLE_HPP
#define SILICIUM_IS_HANDLE_HPP

#include <silicium/type_traits.hpp>
#include <boost/static_assert.hpp>

namespace Si
{
#if SILICIUM_COMPILER_HAS_WORKING_NOEXCEPT
#	define SILICIUM_HAS_IS_HANDLE 1
	template <class T>
	struct is_handle : std::integral_constant<bool,
		Si::is_nothrow_default_constructible<T>::value &&
		Si::is_nothrow_move_assignable<T>::value &&
		Si::is_nothrow_move_constructible<T>::value &&
		Si::is_nothrow_destructible<T>::value
	>
	{
	};
#else
#	define SILICIUM_HAS_IS_HANDLE 0
	template <class T>
	struct is_handle : std::integral_constant<bool,
		!std::is_const<T>::value &&
		!std::is_reference<T>::value &&
		Si::is_default_constructible<T>::value &&
		Si::is_move_assignable<T>::value &&
		Si::is_move_constructible<T>::value
	>
	{
	};
#endif

	BOOST_STATIC_ASSERT(is_handle<char>::value);
	BOOST_STATIC_ASSERT(is_handle<int>::value);
	BOOST_STATIC_ASSERT(is_handle<long>::value);
	BOOST_STATIC_ASSERT(is_handle<float>::value);
	BOOST_STATIC_ASSERT(is_handle<void *>::value);
	BOOST_STATIC_ASSERT(is_handle<int *>::value);
	BOOST_STATIC_ASSERT(is_handle<int (*)()>::value);
	BOOST_STATIC_ASSERT(is_handle<int>::value);
	BOOST_STATIC_ASSERT(!is_handle<int &>::value);
	BOOST_STATIC_ASSERT(!is_handle<int const>::value);
	BOOST_STATIC_ASSERT(is_handle<nothing>::value);
	BOOST_STATIC_ASSERT(is_handle<std::unique_ptr<int>>::value);

	namespace detail
	{
		struct non_copyable
		{
			SILICIUM_DELETED_FUNCTION(non_copyable(non_copyable const &))
		};
		BOOST_STATIC_ASSERT(!is_handle<non_copyable>::value);

#ifndef _MSC_VER
		struct non_assignable
		{
			SILICIUM_DELETED_FUNCTION(non_assignable &operator = (non_assignable const &))
		};
		//VC++ 2013 std::is_{move,copy}_assignable do not return the correct result
		BOOST_STATIC_ASSERT(!Si::is_move_assignable<non_assignable>::value);
#if !SILICIUM_GCC || (SILICIUM_GCC > 406)
		BOOST_STATIC_ASSERT(!Si::is_copy_assignable<non_assignable>::value);
#endif
		BOOST_STATIC_ASSERT(!is_handle<non_assignable>::value);
#endif

#if SILICIUM_COMPILER_HAS_WORKING_NOEXCEPT
		struct non_noexcept_move_constructible
		{
			non_noexcept_move_constructible(non_noexcept_move_constructible &&) BOOST_NOEXCEPT_IF(false);
		};
		BOOST_STATIC_ASSERT(!is_handle<non_noexcept_move_constructible>::value);

		struct non_noexcept_move_assignable
		{
			non_noexcept_move_assignable &operator = (non_noexcept_move_assignable &&) BOOST_NOEXCEPT_IF(false);
		};
		BOOST_STATIC_ASSERT(!is_handle<non_noexcept_move_assignable>::value);
#endif
	}
}

#endif
