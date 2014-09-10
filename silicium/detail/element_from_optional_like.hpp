#ifndef SILICIUM_ELEMENT_FROM_OPTIONAL_LIKE_HPP
#define SILICIUM_ELEMENT_FROM_OPTIONAL_LIKE_HPP

#include <boost/optional.hpp>

namespace Si
{
	namespace detail
	{
		template <class Element>
		struct element_from_optional_like
		{
			using type = Element;
		};

		template <class Element>
		struct element_from_optional_like<boost::optional<Element>>
		{
			using type = Element;
		};
	}
}

#endif
