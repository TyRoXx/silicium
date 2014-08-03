#ifndef SILICIUM_REACTIVE_GET_HPP
#define SILICIUM_REACTIVE_GET_HPP

#include <boost/optional.hpp>
#include <silicium/override.hpp>

namespace rx
{
	template <class Element>
	struct visitor : observer<Element>
	{
		boost::optional<Element> result;

		virtual void got_element(Element value) SILICIUM_OVERRIDE
		{
			result = std::move(value);
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
		}
	};

	template <class Input>
	auto get(Input &from) -> boost::optional<typename Input::element_type>
	{
		visitor<typename Input::element_type> v;
		from.async_get_one(v);
		return std::move(v.result);
	}
}

#endif
