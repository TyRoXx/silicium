#ifndef SILICIUM_REACTIVE_FOR_EACH_HPP
#define SILICIUM_REACTIVE_FOR_EACH_HPP

#include <silicium/total_consumer.hpp>
#include <silicium/transform.hpp>
#include <silicium/config.hpp>
#include <silicium/ptr_observable.hpp>

namespace Si
{
	template <class Input, class Handler>
	auto for_each(Input &&input, Handler &&handle_element)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> total_consumer<unique_observable<nothing>>
#endif
	{
		typedef typename std::decay<Input>::type::element_type element;
		return make_total_consumer(
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			erase_unique
#endif
			(transform(std::forward<Input>(input), [handle_element](element value) -> nothing
		{
			handle_element(std::move(value));
			return {};
		})));
	}
}

#endif
