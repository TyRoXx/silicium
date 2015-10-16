#ifndef SILICIUM_CHECKED_ARITHMETIC_HPP
#define SILICIUM_CHECKED_ARITHMETIC_HPP

#include <silicium/optional.hpp>
#include <limits>
#include <ostream>

namespace Si
{
	struct overflow_type
	{
		BOOST_CONSTEXPR overflow_type() {}
	};

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
		if (value.is_overflow())
		{
			return out << overflow;
		}
		return out << *value.value();
	}

	template <class Unsigned>
	bool operator == (overflow_or<Unsigned> const &left, overflow_or<Unsigned> const &right)
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

	template <class Unsigned>
	overflow_or<Unsigned> checked_add(Unsigned left, Unsigned right)
	{
		Unsigned result = static_cast<Unsigned>(left + right);
		if (result < left)
		{
			return overflow;
		}
		return result;
	}

	template <class Unsigned>
	overflow_or<Unsigned> &operator += (overflow_or<Unsigned> &left, overflow_or<Unsigned> const &right)
	{
		if (left.is_overflow() || right.is_overflow())
		{
			return left;
		}
		left = checked_add(*left.value(), *right.value());
		return left;
	}

	template <class Unsigned>
	overflow_or<Unsigned> operator + (overflow_or<Unsigned> const &left, overflow_or<Unsigned> const &right)
	{
		overflow_or<Unsigned> result = left;
		result += right;
		return result;
	}
}

#endif
