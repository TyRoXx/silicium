#ifndef SILICIUM_DETAIL_PROPER_VALUE_FUNCTION_HPP
#define SILICIUM_DETAIL_PROPER_VALUE_FUNCTION_HPP

#include <functional>

namespace Si
{
	namespace detail
	{
		template <class F, bool IsProperValue, class R, class ...Args>
		struct proper_value_function_impl
		{
			using type = std::function<R (Args...)>;
		};

		template <class F, class R, class ...Args>
		struct proper_value_function_impl<F, true, R, Args...>
		{
			using type = F;
		};

		template <class F, class R, class ...Args>
		using proper_value_function = proper_value_function_impl<F, std::is_default_constructible<F>::value && std::is_move_assignable<F>::value, R, Args...>;
	}
}

#endif
