#ifndef SILICIUM_CONSTANT_OBSERVABLE_HPP
#define SILICIUM_CONSTANT_OBSERVABLE_HPP

#include <silicium/observable/observable.hpp>
#include <silicium/config.hpp>

namespace Si
{
	template <class Element>
	struct constant_observable
	{
		typedef Element element_type;

		constant_observable()
		{
		}

		explicit constant_observable(Element value)
			: value(std::move(value))
		{
		}

		template <class ElementObserver>
		void async_get_one(ElementObserver &receiver) const
		{
			receiver.got_element(value);
		}

	private:

		Element value;
	};

	template <class Element>
	auto make_constant_observable(Element &&value)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> constant_observable<typename std::decay<Element>::type>
#endif
	{
		return constant_observable<typename std::decay<Element>::type>(std::forward<Element>(value));
	}
}

#endif
