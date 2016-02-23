#ifndef SILICIUM_SINK_COPY_HPP
#define SILICIUM_SINK_COPY_HPP

#include <silicium/sink/append.hpp>
#include <silicium/source/source.hpp>

namespace Si
{
	template <class Source, class Sink>
	typename std::enable_if<
	    std::is_convertible<
	        typename std::decay<Source>::type::element_type,
	        typename std::decay<Sink>::type::element_type>::value,
	    void>::type
	copy(Source &&from, Sink &&to)
	{
		for (;;)
		{
			auto element = Si::get(from);
			if (!element)
			{
				break;
			}
			Si::append(to, std::move(*element));
		}
	}
}

#endif
