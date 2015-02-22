#ifndef SILICIUM_FUNCTION_OBSERVABLE_HPP
#define SILICIUM_FUNCTION_OBSERVABLE_HPP

#include <silicium/observable/observer.hpp>
#include <silicium/detail/proper_value_function.hpp>
#include <silicium/detail/element_from_optional_like.hpp>
#include <utility>

namespace Si
{
	template <class Element, class AsyncGetOne>
	struct function_observable
	{
		typedef Element element_type;

		function_observable()
		{
		}

		explicit function_observable(AsyncGetOne get)
			: get(std::move(get))
		{
		}

		void async_get_one(ptr_observer<observer<element_type>> receiver)
		{
			return get(receiver);
		}

	private:

		typedef typename detail::proper_value_function<AsyncGetOne, void, ptr_observer<observer<element_type>>>::type proper_get;

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

	template <class GenerateElement>
	auto make_function_observable2(GenerateElement &&generate)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> function_observable<
			typename detail::element_from_optional_like<typename std::decay<decltype(generate())>::type>::type,
			std::function<void(ptr_observer<observer<typename detail::element_from_optional_like<typename std::decay<decltype(generate())>::type>::type>>)>
		>
#endif
	{
		typedef typename detail::element_from_optional_like<typename std::decay<decltype(generate())>::type>::type element_type;
		return make_function_observable<element_type>(
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			std::function<void(ptr_observer<observer<element_type>>)>
#endif
			([generate
#if SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE
			= std::forward<GenerateElement>(generate)
#endif
			](ptr_observer<observer<element_type>> observer_) mutable
			{
				Si::optional<element_type> element = std::forward<GenerateElement>(generate)();
				if (element)
				{
					observer_.got_element(std::move(*element));
				}
				else
				{
					observer_.ended();
				}
			})
		);
	}
}

#endif
