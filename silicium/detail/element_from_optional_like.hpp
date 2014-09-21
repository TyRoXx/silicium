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
			typedef Element type;
		};

		template <class Element>
		struct element_from_optional_like<boost::optional<Element>>
		{
			typedef Element type;
		};
	}
}

#endif
