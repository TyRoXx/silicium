#ifndef SILICIUM_REACTIVE_DELAY_HPP
#define SILICIUM_REACTIVE_DELAY_HPP

#include <silicium/timer.hpp>
#include <silicium/tuple.hpp>
#include <silicium/ptr_observable.hpp>

namespace Si
{
#if SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
	template <class Input, class Duration>
	auto delay(Input &&input, boost::asio::io_service &io, Duration duration)
	{
		typedef typename std::remove_reference<Input>::type clean_input;
		typedef typename clean_input::element_type element_type;
		auto delaying_timer = erase_shared(timer<>(io, duration));
		auto unpack = [](std::tuple<timer_elapsed, element_type> value)
		{
			return std::move(std::get<1>(value));
		};
		return transform(Si::make_tuple(delaying_timer, std::forward<Input>(input)), unpack);
	}
#endif
}

#endif
