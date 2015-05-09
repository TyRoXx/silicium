#ifndef SILICIUM_MOVE_HPP
#define SILICIUM_MOVE_HPP

#include <silicium/config.hpp>
#include <boost/static_assert.hpp>

namespace Si
{
	template <class T>
	typename std::remove_reference<T>::type &&move(T &&ref)
	{
		BOOST_STATIC_ASSERT(!std::is_const<T>::value);
		return static_cast<typename std::remove_reference<T>::type &&>(ref);
	}
}

#endif
