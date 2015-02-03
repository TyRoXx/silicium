#ifndef SILICIUM_THROWING_SOURCE_HPP
#define SILICIUM_THROWING_SOURCE_HPP

#include <silicium/source/transforming_source.hpp>

namespace Si
{
	template <class SourceOfErrorOrs>
	auto make_throwing_source(SourceOfErrorOrs &&input)
	{
		return make_transforming_source(
			std::forward<SourceOfErrorOrs>(input),
			[](typename std::decay<SourceOfErrorOrs>::type::element_type element)
		{
			return std::move(element).get();
		});
	}
}

#endif
