#ifndef SILICIUM_DETAIL_PROPER_VALUE_FUNCTION_HPP
#define SILICIUM_DETAIL_PROPER_VALUE_FUNCTION_HPP

#include <functional>

namespace Si
{
	namespace detail
	{
		template <class F, class R, class ...Args>
		using proper_value_function = std::conditional<
			std::is_default_constructible<F>::value && std::is_move_assignable<F>::value,
			F,
			std::function<R (Args...)>
		>;
	}
}

#endif
