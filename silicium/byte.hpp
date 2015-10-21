#ifndef SILICIUM_BYTE_HPP
#define SILICIUM_BYTE_HPP

#include <boost/cstdint.hpp>
#include <boost/functional/hash.hpp>

namespace Si
{
	enum class byte : boost::uint8_t
	{
		minimum = 0,
		zero = 0,
		one = 1,
		maximum = 255
	};

	inline std::size_t hash_value(byte value)
	{
		using boost::hash_value;
		return hash_value(static_cast<boost::uint8_t>(value));
	}

	template <class Char>
	std::basic_ostream<Char> &operator<<(std::basic_ostream<Char> &out, byte value)
	{
		return out << static_cast<unsigned>(value);
	}
}

namespace std
{
	template <>
	struct hash<Si::byte>
	{
		std::size_t operator()(Si::byte value) const
		{
			return hash_value(value);
		}
	};
}

#endif
