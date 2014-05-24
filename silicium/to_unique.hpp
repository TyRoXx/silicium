#ifndef SILICIUM_TO_UNIQUE_HPP
#define SILICIUM_TO_UNIQUE_HPP

#include <memory>

namespace Si
{
	template <class T>
	std::unique_ptr<T> to_unique(T value)
	{
		return std::unique_ptr<T>(new T(std::move(value)));
	}
}

#endif
