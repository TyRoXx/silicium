#ifndef SILICIUM_MOVE_IF_NOEXCEPT_HPP
#define SILICIUM_MOVE_IF_NOEXCEPT_HPP

#include <silicium/config.hpp>
#include <utility>

namespace Si
{
#define SILICIUM_HAS_MOVE_IF_NOEXCEPT SILICIUM_COMPILER_HAS_WORKING_NOEXCEPT

#if SILICIUM_HAS_MOVE_IF_NOEXCEPT

#if defined(__GNUC__) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 407) || defined(__clang__) || (defined(_MSC_VER) && (_MSC_VER >= 1800))
	using std::move_if_noexcept;
#else
	template <class T, class Result = typename std::conditional<
		std::is_nothrow_constructible<T, T>::value,
		const T &,
		T &&
	>::type>
	BOOST_CONSTEXPR Result move_if_noexcept(T &x)
	{
		return static_cast<Result>(x);
	}
#endif

#endif
}

#endif
