#ifndef SILICIUM_REACTIVE_DEREF_OPTIONAL_HPP
#define SILICIUM_REACTIVE_DEREF_OPTIONAL_HPP

#include <boost/optional.hpp>
#include <silicium/observable/ptr_observable.hpp>
#include <silicium/config.hpp>

namespace Si
{
	template <class Input>
	auto deref_optional(Input &&input)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> ptr_observable<typename Input::element_type, std::unique_ptr<typename Input::element_type>>
#endif
	{
		typedef boost::optional<typename Input::element_type> optional_type;
		auto is_set = [](optional_type const &element) { return element.is_initialized(); };
		auto deref = [](optional_type element) { return std::move(*element); };
		return
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			box
#endif
			(transform(while_(std::forward<Input>(input), is_set), deref));
	}
}

#endif
