#ifndef SILICIUM_MAKE_ARRAY_HPP
#define SILICIUM_MAKE_ARRAY_HPP

#include <array>

namespace Si
{
	template <class ...T>
	auto make_array(T &&...elements)
	{
		std::array<typename std::decay<typename std::common_type<T...>::type>::type, sizeof...(elements)> result =
		{{
			std::forward<T>(elements)...
		}};
		return result;
	}

	template <class Element, class ...T>
	auto make_array(T &&...elements)
	{
		std::array<Element, sizeof...(elements)> result =
		{{
			std::forward<T>(elements)...
		}};
		return result;
	}
}

#endif
