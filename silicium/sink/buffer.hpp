#ifndef SILICIUM_SINK_BUFFER_HPP
#define SILICIUM_SINK_BUFFER_HPP

#include <silicium/trait.hpp>
#include <silicium/iterator_range.hpp>

namespace Si
{
	template <class Element, class Error>
	SILICIUM_TRAIT_WITH_TYPEDEFS(Buffer, typedef Element element_type; typedef Error error_type;
	                             , ((make_append_space, (1, (std::size_t)),
	                                 iterator_range<element_type *>))((flush_append_space, (0), error_type)))

#if SILICIUM_COMPILER_HAS_USING
	template <class Element, class Error>
	using buffer = typename Buffer<Element, Error>::interface;
#endif
}

#endif
