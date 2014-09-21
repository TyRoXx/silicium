#ifndef SILICIUM_DETAIL_PROPER_VALUE_FUNCTION_HPP
#define SILICIUM_DETAIL_PROPER_VALUE_FUNCTION_HPP

#include <silicium/config.hpp>
#include <functional>

namespace Si
{
	namespace detail
	{
		template <class F, class R, class ...Args>
		struct proper_value_function : std::conditional<
			Si::is_default_constructible<F>::value && Si::is_move_assignable<F>::value,
			F,
			std::function<R (Args...)>
		>
		{
		};
	}
}

#endif
