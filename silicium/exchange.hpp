#ifndef SILICIUM_REACTIVE_EXCHANGE_HPP
#define SILICIUM_REACTIVE_EXCHANGE_HPP

#include <utility>

namespace Si
{
	template <class T, class U>
	T exchange(T &dest, U &&source)
	{
		auto old = dest;
		dest = std::forward<U>(source);
		return old;
	}
}

#endif
