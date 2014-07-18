#ifndef SILICIUM_REACTIVE_FOR_EACH_HPP
#define SILICIUM_REACTIVE_FOR_EACH_HPP

#include <reactive/total_consumer.hpp>
#include <reactive/transform.hpp>
#include <reactive/config.hpp>

namespace rx
{
	template <class Input, class Handler>
	auto for_each(Input &&input, Handler &&handle_element)
	{
		typedef typename std::decay<Input>::type::element_type element;
		return make_total_consumer(rx::transform(std::forward<Input>(input), [handle_element](element value) -> detail::nothing
		{
			handle_element(std::move(value));
			return {};
		}));
	}
}

#endif
