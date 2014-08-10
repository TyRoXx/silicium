#ifndef SILICIUM_REACTIVE_TAKE_HPP
#define SILICIUM_REACTIVE_TAKE_HPP

#include <silicium/observable.hpp>
#include <boost/optional.hpp>

namespace Si
{
	template <class Element>
	struct optional_observer : observer<Element>
	{
		boost::optional<Element> element;

		virtual void got_element(Element value) SILICIUM_OVERRIDE
		{
			element = std::move(value);
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			element.reset();
		}
	};

	template <class Element>
	boost::optional<Element> get_immediate(observable<Element> &from)
	{
		optional_observer<Element> current;
		from.async_get_one(current);
		return std::move(current.element);
	}

	template <class Element>
	std::vector<Element> take(observable<Element> &from, std::size_t count)
	{
		std::vector<Element> taken;
		for (std::size_t i = 0; i < count; ++i)
		{
			auto current = get_immediate(from);
			if (!current)
			{
				break;
			}
			taken.emplace_back(std::move(*current));
		}
		return taken;
	}
}

#endif
