#ifndef SILICIUM_FUNCTION_OBSERVER_HPP
#define SILICIUM_FUNCTION_OBSERVER_HPP

#include <silicium/observable/observer.hpp>
#include <boost/optional.hpp>

namespace Si
{
	namespace detail
	{
		template <class Element>
		struct optional_maker
		{
			typename std::decay<Element>::type *value;

			template <class T>
			operator boost::optional<T>() const
			{
				return static_cast<T>(std::forward<Element>(*value));
			}
		};
	}

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
			m_function(detail::optional_maker<Element>{&element});
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
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> function_observer<typename std::decay<Function>::type>
#endif
	{
		return function_observer<typename std::decay<Function>::type>(std::forward<Function>(function));
	}
}

#endif
