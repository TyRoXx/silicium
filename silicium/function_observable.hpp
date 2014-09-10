#ifndef SILICIUM_FUNCTION_OBSERVABLE_HPP
#define SILICIUM_FUNCTION_OBSERVABLE_HPP

#include <silicium/observer.hpp>
#include <silicium/detail/proper_value_function.hpp>
#include <utility>

namespace Si
{
	template <class Element, class AsyncGetOne>
	struct function_observable
	{
		using element_type = Element;

		function_observable()
		{
		}

		explicit function_observable(AsyncGetOne get)
			: get(std::move(get))
		{
		}

		void async_get_one(observer<element_type> &receiver)
		{
			return get(receiver);
		}

	private:

		using proper_get = typename detail::proper_value_function<AsyncGetOne, void, observer<element_type> &>::type;

		proper_get get;
	};

	template <class Element, class AsyncGetOne>
	auto make_function_observable(AsyncGetOne &&get)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> function_observable<Element, typename std::decay<AsyncGetOne>::type>
#endif
	{
		return function_observable<Element, typename std::decay<AsyncGetOne>::type>(std::forward<AsyncGetOne>(get));
	}
}

#endif
