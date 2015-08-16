#ifndef SILICIUM_SAFE_ARITHMETIC_HPP
#define SILICIUM_SAFE_ARITHMETIC_HPP

#include <silicium/optional.hpp>
#include <limits>
#include <ostream>

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

	template <class T>
	safe_number<T> safe(T value)
	{
		return std::move(value);
	}

	struct overflow_type {};

	template <class Char, class Traits>
	std::basic_ostream<Char, Traits> &operator << (std::basic_ostream<Char, Traits> &out, overflow_type)
	{
		return out << "overflow";
	}

	static BOOST_CONSTEXPR_OR_CONST overflow_type overflow;

	template <class Unsigned>
	struct overflow_or
	{
		typedef Unsigned value_type;

		overflow_or() BOOST_NOEXCEPT
		{
		}

		overflow_or(Unsigned value)
			: m_state(value)
		{
		}

		overflow_or(overflow_type)
		{
		}

		bool is_overflow() const BOOST_NOEXCEPT
		{
			return !m_state;
		}

		optional<Unsigned> const &value() const BOOST_NOEXCEPT
		{
			return m_state;
		}

	private:

		optional<Unsigned> m_state;
	};

	template <class Char, class Traits, class T>
	std::basic_ostream<Char, Traits> &operator << (std::basic_ostream<Char, Traits> &out, overflow_or<T> const &value)
	{
		return out << value.value();
	}

	template <class Unsigned>
	overflow_or<safe_number<Unsigned>> operator + (safe_number<Unsigned> left, safe_number<Unsigned> right)
	{
		safe_number<Unsigned> result;
		result.value = static_cast<Unsigned>(left.value + right.value);
		if (result.value < left.value)
		{
			return overflow;
		}
		return result;
	}

	template <class Unsigned>
	overflow_or<safe_number<Unsigned>> operator - (safe_number<Unsigned> left, safe_number<Unsigned> right)
	{
		if (left.value < right.value)
		{
			return overflow;
		}
		safe_number<Unsigned> result;
		result.value = left.value - right.value;
		return result;
	}

	template <class Unsigned>
	overflow_or<safe_number<Unsigned>> operator * (safe_number<Unsigned> left, safe_number<Unsigned> right)
	{
		safe_number<Unsigned> result;
		if (left.value != 0 && (((std::numeric_limits<Unsigned>::max)() / left.value) < right.value))
		{
			return overflow;
		}
		result.value = static_cast<Unsigned>(left.value * right.value);
		return result;
	}

	template <class Unsigned>
	overflow_or<safe_number<Unsigned>> operator / (safe_number<Unsigned> left, safe_number<Unsigned> right)
	{
		safe_number<Unsigned> result;
		if (right.value == 0)
		{
			return overflow;
		}
		result.value = left.value / right.value;
		return result;
	}

#define SILICIUM_SAFE_NUMBER_DEFINE_OPTIONAL_OPERATOR(op) \
	template <class Unsigned> \
	overflow_or<safe_number<Unsigned>> operator op (overflow_or<safe_number<Unsigned>> left, safe_number<Unsigned> right) \
	{ \
		if (left.is_overflow()) \
		{ \
			return overflow; \
		} \
		return *left.value() op right; \
	} \
	template <class Unsigned> \
	overflow_or<safe_number<Unsigned>> operator op (safe_number<Unsigned> left, overflow_or<safe_number<Unsigned>> right) \
	{ \
		if (right.is_overflow()) \
		{ \
			return overflow; \
		} \
		return left op *right.value(); \
	} \
	template <class Unsigned> \
	overflow_or<safe_number<Unsigned>> operator op (overflow_or<safe_number<Unsigned>> left, overflow_or<safe_number<Unsigned>> right) \
	{ \
		if (left.is_overflow() || right.is_overflow()) \
		{ \
			return overflow; \
		} \
		return *left.value() op *right.value(); \
	} \
	template <class Unsigned> \
	overflow_or<safe_number<Unsigned>> operator op (overflow_or<safe_number<Unsigned>> const &, overflow_type) \
	{ \
		return overflow; \
	} \
	template <class Unsigned> \
	overflow_or<safe_number<Unsigned>> operator op (safe_number<Unsigned> const &, overflow_type) \
	{ \
		return overflow; \
	} \
	template <class Unsigned> \
	overflow_or<safe_number<Unsigned>> operator op (overflow_type, overflow_or<safe_number<Unsigned>> const &) \
	{ \
		return overflow; \
	} \
	template <class Unsigned> \
	overflow_or<safe_number<Unsigned>> operator op (overflow_type, safe_number<Unsigned> const &) \
	{ \
		return overflow; \
	}

	SILICIUM_SAFE_NUMBER_DEFINE_OPTIONAL_OPERATOR(+)
	SILICIUM_SAFE_NUMBER_DEFINE_OPTIONAL_OPERATOR(-)
	SILICIUM_SAFE_NUMBER_DEFINE_OPTIONAL_OPERATOR(*)
	SILICIUM_SAFE_NUMBER_DEFINE_OPTIONAL_OPERATOR(/)
#undef SILICIUM_SAFE_NUMBER_DEFINE_OPTIONAL_OPERATOR

	template <class T>
	bool operator == (safe_number<T> const &left, safe_number<T> const &right)
	{
		return left.value == right.value;
	}

	template <class T>
	bool operator == (overflow_or<safe_number<T>> const &left, overflow_type)
	{
		return left.is_overflow();
	}

	template <class T>
	bool operator == (overflow_type, overflow_or<safe_number<T>> const &right)
	{
		return right.is_overflow();
	}

	template <class T>
	bool operator == (overflow_or<safe_number<T>> const &left, safe_number<T> const &right)
	{
		if (left.is_overflow())
		{
			return false;
		}
		return *left.value() == right;
	}

	template <class T>
	bool operator == (safe_number<T> const &left, overflow_or<safe_number<T>> const &right)
	{
		if (right.is_overflow())
		{
			return false;
		}
		return left == *right.value();
	}

	template <class T>
	bool operator == (overflow_or<safe_number<T>> const &left, overflow_or<safe_number<T>> const &right)
	{
		if (left.is_overflow() && right.is_overflow())
		{
			return true;
		}
		if (left.is_overflow() != right.is_overflow())
		{
			return false;
		}
		return *left.value() == *right.value();
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

	template <class Char, class Traits, class T>
	std::basic_ostream<Char, Traits> &operator << (std::basic_ostream<Char, Traits> &out, safe_number<T> const &value)
	{
		return out << value.value;
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
