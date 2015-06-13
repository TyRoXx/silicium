#ifndef SILICIUM_MAKE_ARRAY_HPP
#define SILICIUM_MAKE_ARRAY_HPP

#include <silicium/config.hpp>
#include <silicium/identity.hpp>
#include <array>

namespace Si
{
	template <class Element = void, class ...T>
	auto make_array(T &&...elements)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> std::array<
			typename std::conditional<
				std::is_same<Element, void>::value,
				std::common_type<T...>,
				identity<Element>
			>::type::type,
			sizeof...(elements)
		>
#endif
	{
		typedef typename std::conditional<
			std::is_same<Element, void>::value,
			std::common_type<T...>,
			identity<Element>
		>::type::type element_type;
		std::array<element_type, sizeof...(elements)> result =
		{{
			std::forward<T>(elements)...
		}};
		return result;
	}
}

#endif
