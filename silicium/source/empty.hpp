#ifndef SILICIUM_EMPTY_SOURCE_HPP
#define SILICIUM_EMPTY_SOURCE_HPP

#include <silicium/source/source.hpp>
#include <vector>
#include <string>

namespace Si
{
	template <class Element>
	struct empty_source SILICIUM_FINAL : source<Element>
	{
		virtual iterator_range<Element const *> map_next(std::size_t size) SILICIUM_OVERRIDE
		{
			boost::ignore_unused_variable_warning(size);
			return {};
		}

		virtual Element *copy_next(iterator_range<Element *> destination) SILICIUM_OVERRIDE
		{
			return destination.begin();
		}
	};
}

#endif
