#ifndef SILICIUM_IS_HANDLE_HPP
#define SILICIUM_IS_HANDLE_HPP

#include <silicium/config.hpp>
#include <boost/static_assert.hpp>

namespace Si
{
	template <class T>
	struct is_handle : std::integral_constant<bool,
		Si::is_nothrow_default_constructible<T>::value &&
		Si::is_nothrow_move_assignable<T>::value &&
		Si::is_nothrow_move_constructible<T>::value &&
		std::is_nothrow_destructible<T>::value
	>
	{
	};

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

		struct non_assignable
		{
			SILICIUM_DELETED_FUNCTION(non_assignable &operator = (non_assignable const &))
		};
		BOOST_STATIC_ASSERT(!is_handle<non_assignable>::value);

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
	}
}

#endif
