#ifndef SILICIUM_BYTE_HPP
#define SILICIUM_BYTE_HPP

#include <boost/cstdint.hpp>

namespace Si
{
	enum class byte : boost::uint8_t
	{
		minimum = 0,
		zero = 0,
		one = 1,
		maximum = 255
	};
}

#endif
