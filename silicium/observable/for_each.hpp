#ifndef SILICIUM_REACTIVE_FOR_EACH_HPP
#define SILICIUM_REACTIVE_FOR_EACH_HPP

#include <silicium/observable/total_consumer.hpp>
#include <silicium/observable/transform.hpp>
#include <silicium/config.hpp>
#include <silicium/observable/ptr.hpp>
#include <silicium/observable/erase_unique.hpp>

#define SILICIUM_HAS_FOR_EACH_OBSERVABLE SILICIUM_HAS_TRANSFORM_OBSERVABLE

namespace Si
{
#if SILICIUM_HAS_FOR_EACH_OBSERVABLE
	template <class Input, class Handler>
	auto for_each(Input &&input, Handler &&handle_element)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
	    -> total_consumer<unique_observable<unit>>
#endif
	{
		typedef typename std::decay<Input>::type::element_type element;
#ifdef _MSC_VER
		// VC++ cannot capture arbitrary lambda closures by value, but we can work around this
		// limitation by wrapping the closure in a std::function.
		std::function<void(element)> handle_element_capture(std::forward<Handler>(handle_element));
#else
		auto &&handle_element_capture = handle_element;
#endif
		return make_total_consumer(
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		    erase_unique
#endif
		    (transform(std::forward<Input>(input), [handle_element_capture](element value) -> unit
		               {
			               handle_element_capture(std::move(value));
			               return unit();
			           })));
	}
#endif
}

#endif
