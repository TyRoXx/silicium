#ifndef SILICIUM_FUNCTION_OBSERVER_HPP
#define SILICIUM_FUNCTION_OBSERVER_HPP

#include <silicium/observable/observer.hpp>

namespace Si
{
	template <class Function>
	struct function_observer
	{
		explicit function_observer(Function function)
			: m_function(std::move(function))
		{
		}

		template <class Element>
		void got_element(Element &&element)
		{
			m_function(std::forward<Element>(element));
		}

		void ended()
		{
			m_function(boost::none);
		}

	private:

		Function m_function;
	};

	template <class Function>
	auto make_function_observer(Function &&function)
	{
		return function_observer<typename std::decay<Function>::type>(std::forward<Function>(function));
	}
}

#endif
