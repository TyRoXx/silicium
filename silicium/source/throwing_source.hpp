#ifndef SILICIUM_THROWING_SOURCE_HPP
#define SILICIUM_THROWING_SOURCE_HPP

#include <silicium/source/transforming_source.hpp>

#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
#include <silicium/source/ptr_source.hpp>
#endif

namespace Si
{
	template <class SourceOfErrorOrs>
	auto make_throwing_source(SourceOfErrorOrs &&input)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
	    -> ptr_source<std::unique_ptr<
	        typename Source<typename std::decay<SourceOfErrorOrs>::type::element_type::value_type>::interface>>
#endif
	{
		return
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		    erase_source
#endif
		    (make_transforming_source(std::forward<SourceOfErrorOrs>(input),
		                              [](typename std::decay<SourceOfErrorOrs>::type::element_type element)
		                              {
			                              return std::move(element).get();
			                          }));
	}
}

#endif
