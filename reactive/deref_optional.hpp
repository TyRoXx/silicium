#ifndef SILICIUM_REACTIVE_DEREF_OPTIONAL_HPP
#define SILICIUM_REACTIVE_DEREF_OPTIONAL_HPP

#include <boost/optional.hpp>

namespace rx
{
	template <class Input>
	auto deref_optional(Input &&input)
	{
		typedef boost::optional<typename Input::element_type> optional_type;
		auto is_set = [](optional_type const &element) { return element.is_initialized(); };
		auto deref = [](optional_type element) { return std::move(*element); };
		return transform(while_(std::forward<Input>(input), is_set), deref);
	}
}

#endif
