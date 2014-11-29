#ifndef SILICIUM_REACTIVE_EXCHANGE_HPP
#define SILICIUM_REACTIVE_EXCHANGE_HPP

#include <utility>

namespace Si
{
	namespace detail
	{
		template <class T, class U>
		T exchange_impl(T &dest, U &&source, std::true_type)
		{
			auto old = std::move_if_noexcept(dest);
			dest = std::forward<U>(source);
			return old;
		}

		template <class T, class U>
		T exchange_impl(T &dest, U &&source, std::false_type)
		{
			auto old = dest;
			dest = std::forward<U>(source);
			return old;
		}
	}

	template <class T, class U>
	T exchange(T &dest, U &&source)
	{
		return detail::exchange_impl(dest, std::forward<U>(source), std::integral_constant<bool, noexcept(dest = std::forward<U>(source))>());
	}
}

#endif
