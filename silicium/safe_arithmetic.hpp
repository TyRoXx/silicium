#ifndef SILICIUM_SAFE_ARITHMETIC_HPP
#define SILICIUM_SAFE_ARITHMETIC_HPP

#include <silicium/optional.hpp>
#include <limits>

namespace Si
{
	template <class Unsigned>
	struct safe_number
	{
		BOOST_STATIC_ASSERT(std::is_unsigned<Unsigned>::value);

		Unsigned value;

		safe_number()
			// default constructs to zero
		    : value()
		{
		}

		safe_number(Unsigned value)
		    : value(value)
		{
		}
	};

	template <class Unsigned>
	optional<safe_number<Unsigned>> operator + (safe_number<Unsigned> left, safe_number<Unsigned> right)
	{
		safe_number<Unsigned> result;
		result.value = left.value + right.value;
		if (result.value < left.value)
		{
			return none;
		}
		return result;
	}

	template <class Unsigned>
	optional<safe_number<Unsigned>> operator - (safe_number<Unsigned> left, safe_number<Unsigned> right)
	{
		if (left.value < right.value)
		{
			return none;
		}
		safe_number<Unsigned> result;
		result.value = left.value - right.value;
		return result;
	}

	template <class Unsigned>
	optional<safe_number<Unsigned>> operator * (safe_number<Unsigned> left, safe_number<Unsigned> right)
	{
		safe_number<Unsigned> result;
		result.value = left.value * right.value;
		if (left.value != 0 && (((std::numeric_limits<Unsigned>::max)() / left.value) < right.value))
		{
			return none;
		}
		return result;
	}

#define SILICIUM_SAFE_NUMBER_DEFINE_OPTIONAL_OPERATOR(op) \
	template <class Unsigned> \
	optional<safe_number<Unsigned>> operator op (optional<safe_number<Unsigned>> left, safe_number<Unsigned> right) \
	{ \
		if (!left) \
		{ \
			return none; \
		} \
		return *left op right; \
	} \
	template <class Unsigned> \
	optional<safe_number<Unsigned>> operator op (safe_number<Unsigned> left, optional<safe_number<Unsigned>> right) \
	{ \
		if (!right) \
		{ \
			return none; \
		} \
		return left op *right; \
	} \
	template <class Unsigned> \
	optional<safe_number<Unsigned>> operator op (optional<safe_number<Unsigned>> left, optional<safe_number<Unsigned>> right) \
	{ \
		if (!left || !right) \
		{ \
			return none; \
		} \
		return *left op *right; \
	}

	SILICIUM_SAFE_NUMBER_DEFINE_OPTIONAL_OPERATOR(+)
	SILICIUM_SAFE_NUMBER_DEFINE_OPTIONAL_OPERATOR(-)
	SILICIUM_SAFE_NUMBER_DEFINE_OPTIONAL_OPERATOR(*)
#undef SILICIUM_SAFE_NUMBER_DEFINE_OPTIONAL_OPERATOR

	template <class T>
	bool operator == (safe_number<T> const &left, safe_number<T> const &right)
	{
		return left.value == right.value;
	}

	template <class T>
	bool operator != (safe_number<T> const &left, safe_number<T> const &right)
	{
		return left.value != right.value;
	}

	template <class T>
	bool operator < (safe_number<T> const &left, safe_number<T> const &right)
	{
		return left.value < right.value;
	}

	template <class T>
	std::size_t hash_value(safe_number<T> const &value)
	{
		using boost::hash_value;
		return hash_value(value.value);
	}
}

namespace std
{
	template <class T>
	struct hash<Si::safe_number<T>>
	{
		std::size_t operator()(Si::safe_number<T> const &value) const
		{
			return hash_value(value);
		}
	};
}

#endif
