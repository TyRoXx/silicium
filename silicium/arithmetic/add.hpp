#ifndef SILICIUM_ARITHMETIC_ADD_HPP
#define SILICIUM_ARITHMETIC_ADD_HPP

#include <silicium/arithmetic/overflow_or.hpp>

namespace Si
{
    template <class Unsigned>
    typename std::enable_if<std::is_unsigned<Unsigned>::value,
                            overflow_or<Unsigned>>::type
    checked_add(Unsigned left, Unsigned right)
    {
        Unsigned result = static_cast<Unsigned>(left + right);
        if (result < left)
        {
            return overflow;
        }
        return result;
    }

    template <class Number>
    overflow_or<Number> &operator+=(overflow_or<Number> &left,
                                    overflow_or<Number> const &right)
    {
        if (left.is_overflow() || right.is_overflow())
        {
            return left;
        }
        using ::Si::checked_add;
        left = checked_add(*left.value(), *right.value());
        return left;
    }

    template <class Number>
    overflow_or<Number> &operator+=(overflow_or<Number> &left,
                                    Number const &right)
    {
        if (left.is_overflow())
        {
            return left;
        }
        using ::Si::checked_add;
        left = checked_add(*left.value(), right);
        return left;
    }

    template <class Number>
    overflow_or<Number> operator+(overflow_or<Number> const &left,
                                  overflow_or<Number> const &right)
    {
        overflow_or<Number> result = left;
        result += right;
        return result;
    }

    template <class Number>
    overflow_or<Number> operator+(Number const &left,
                                  overflow_or<Number> const &right)
    {
        overflow_or<Number> result = right;
        result += left;
        return result;
    }

    template <class Number>
    overflow_or<Number> operator+(overflow_or<Number> const &left,
                                  Number const &right)
    {
        overflow_or<Number> result = left;
        result += right;
        return result;
    }
}

#endif
