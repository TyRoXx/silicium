#ifndef SILICIUM_REACTIVE_DEREF_OPTIONAL_HPP
#define SILICIUM_REACTIVE_DEREF_OPTIONAL_HPP

#include <boost/optional.hpp>
#ifdef _MSC_VER
#include <silicium/ptr_observable.hpp>
#endif

namespace Si
{
	template <class Input>
	auto deref_optional(Input &&input) -> ptr_observable<typename Input::element_type, std::unique_ptr<typename Input::element_type>>
	{
		typedef boost::optional<typename Input::element_type> optional_type;
		auto is_set = [](optional_type const &element) { return element.is_initialized(); };
		auto deref = [](optional_type element) { return std::move(*element); };
		return
#ifdef _MSC_VER
			box
#endif
			(transform(while_(std::forward<Input>(input), is_set), deref));
	}
}

#endif
