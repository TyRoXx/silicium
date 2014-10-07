#ifndef SILICIUM_COPY_HPP
#define SILICIUM_COPY_HPP

#include <silicium/sink.hpp>
#include <silicium/source.hpp>

namespace Si
{
	template <class T, class Error>
	void copy(source<T> &from, sink<T, Error> &to)
	{
		for (;;)
		{
			buffering_sink<T, Error, std::array<T, 1>> buffer(to);
			auto space = buffer.make_append_space(1);
			auto copied_until = from.copy_next(space);
			if (copied_until == space.begin())
			{
				//end of input
				break;
			}
			//TODO: handle error
			buffer.flush_append_space();
		}
	}
}

#endif
