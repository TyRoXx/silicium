#ifndef SILICIUM_BOUNDED_INT_HPP
#define SILICIUM_BOUNDED_INT_HPP

#include <silicium/optional.hpp>

namespace Si
{
    namespace detail
    {
        template <class Int, Int Minimum, Int Maximum>
        struct compact_int
        {
            constexpr explicit compact_int(Int value)
                : m_value(value)
            {
            }

            constexpr Int value() const
            {
                return m_value;
            }

        private:
            Int m_value;
        };

        // no need to store the value if there is only one possibility
        template <class Int, Int Value>
        struct compact_int<Int, Value, Value>
        {
            constexpr explicit compact_int(Int)
            {
            }

            constexpr static Int value()
            {
                return Value;
            }
        };
    }

    template <class Int, Int Minimum, Int Maximum>
    struct bounded_int : private detail::compact_int<Int, Minimum, Maximum>
    {
        typedef Int value_type;

        BOOST_STATIC_ASSERT(Minimum <= Maximum);

        template <Int OtherMinimum, Int OtherMaximum>
        bounded_int(bounded_int<Int, OtherMinimum, OtherMaximum> const &other)
            : value_base(other.value())
        {
            BOOST_STATIC_ASSERT(OtherMinimum >= Minimum);
            BOOST_STATIC_ASSERT(OtherMaximum <= Maximum);
        }

        static optional<bounded_int> create(Int possible_value)
        {
            value_type minimum = Minimum;
            if (possible_value < minimum)
            {
                return none;
            }
            value_type maximum = Maximum;
            if (possible_value > maximum)
            {
                return none;
            }
            return bounded_int(possible_value);
        }

        constexpr static bounded_int create_or_throw(Int possible_value)
        {
            return (possible_value < Minimum)
                       ? throw std::out_of_range("bounded_int")
                       : ((possible_value > Maximum)
                              ? throw std::out_of_range("bounded_int")
                              : bounded_int(possible_value));
        }

        template <class OtherInt, OtherInt OtherMinimum, OtherInt OtherMaximum>
        static optional<bounded_int>
        create(bounded_int<OtherInt, OtherMinimum, OtherMaximum> possible_value)
        {
            value_type minimum = Minimum;
            if (possible_value.value() < minimum)
            {
                return none;
            }
            value_type maximum = Maximum;
            if (possible_value.value() > maximum)
            {
                return none;
            }
            return bounded_int(static_cast<Int>(possible_value.value()));
        }

        template <Int Literal>
        constexpr static bounded_int literal()
        {
            BOOST_STATIC_ASSERT(Literal >= Minimum);
            BOOST_STATIC_ASSERT(Literal <= Maximum);
            return bounded_int(Literal);
        }

        constexpr Int value() const
        {
            return value_base::value();
        }

        template <Int NewMinimum, Int NewMaximum>
        bounded_int<Int, NewMinimum, NewMaximum> clamp() const
        {
            using std::min;
            using std::max;
            auto result = bounded_int<Int, NewMinimum, NewMaximum>::create(
                max(NewMinimum, min(NewMaximum, value())));
            assert(result);
            return *result;
        }

        template <Int NewMinimum, Int NewMaximum>
        optional<bounded_int<Int, NewMinimum, NewMaximum>> narrow() const
        {
            return bounded_int<Int, NewMinimum, NewMaximum>::create(value());
        }

    private:
        typedef detail::compact_int<Int, Minimum, Maximum> value_base;

        constexpr explicit bounded_int(Int value)
            : value_base(value)
        {
        }
    };

    template <class Int, Int MinimumLeft, Int MaximumLeft, Int MinimumRight,
              Int MaximumRight>
    bool operator==(bounded_int<Int, MinimumLeft, MaximumLeft> const &left,
                    bounded_int<Int, MinimumRight, MaximumRight> const &right)
    {
        BOOST_STATIC_ASSERT(
            ((MaximumLeft > MinimumRight) && (MinimumLeft <= MinimumRight)) ||
            ((MaximumRight > MinimumLeft) && (MinimumRight <= MinimumLeft)));
        return left.value() == right.value();
    }

    template <class Int, Int MinimumLeft, Int MaximumLeft, Int MinimumRight,
              Int MaximumRight>
    bool operator<(bounded_int<Int, MinimumLeft, MaximumLeft> const &left,
                   bounded_int<Int, MinimumRight, MaximumRight> const &right)
    {
        BOOST_STATIC_ASSERT(
            ((MaximumLeft > MinimumRight) && (MinimumLeft <= MinimumRight)) ||
            ((MaximumRight > MinimumLeft) && (MinimumRight <= MinimumLeft)));
        return left.value() < right.value();
    }

    template <class Int, Int Minimum, Int Maximum>
    std::ostream &operator<<(std::ostream &out,
                             bounded_int<Int, Minimum, Maximum> const &value)
    {
        // Propagate char types to int by adding zero so that ostream will
        // properly format them as numbers.
        auto printable = (0 + value.value());
        return out << printable;
    }

    template <class Int, Int MinimumLeft, Int MaximumLeft, Int MinimumRight,
              Int MaximumRight>
    bounded_int<Int, MinimumLeft + MinimumRight, MaximumLeft + MaximumRight>
    operator+(bounded_int<Int, MinimumLeft, MaximumLeft> const &left,
              bounded_int<Int, MinimumRight, MaximumRight> const &right)
    {
        auto result =
            bounded_int<Int, MinimumLeft + MinimumRight,
                        MaximumLeft + MaximumRight>::create(left.value() +
                                                            right.value());
        assert(result);
        return *result;
    }

    template <class Int, Int MinimumLeft, Int MaximumLeft, Int MinimumRight,
              Int MaximumRight>
    constexpr bounded_int<Int, MinimumLeft * MinimumRight,
                          MaximumLeft * MaximumRight>
    operator*(bounded_int<Int, MinimumLeft, MaximumLeft> const &left,
              bounded_int<Int, MinimumRight, MaximumRight> const &right)
    {
        return bounded_int<Int, MinimumLeft * MinimumRight,
                           MaximumLeft *
                               MaximumRight>::create_or_throw(left.value() *
                                                              right.value());
    }

    template <class Left, class Right>
    struct is_always_less;

    template <class Int, Int MinimumLeft, Int MaximumLeft, Int MinimumRight,
              Int MaximumRight>
    struct is_always_less<bounded_int<Int, MinimumLeft, MaximumLeft>,
                          bounded_int<Int, MinimumRight, MaximumRight>>
        : std::integral_constant<bool, (MaximumLeft < MinimumRight)>
    {
        BOOST_STATIC_ASSERT(MinimumLeft <= MaximumLeft);
        BOOST_STATIC_ASSERT(MinimumRight <= MaximumRight);
    };

    BOOST_STATIC_ASSERT(
        is_always_less<bounded_int<int, 0, 0>, bounded_int<int, 1, 2>>::value);
    BOOST_STATIC_ASSERT(is_always_less<bounded_int<unsigned, 0, 0>,
                                       bounded_int<unsigned, 1, 2>>::value);

    BOOST_STATIC_ASSERT(
        !is_always_less<bounded_int<int, 0, 0>, bounded_int<int, 0, 2>>::value);
    BOOST_STATIC_ASSERT(!is_always_less<bounded_int<unsigned, 0, 0>,
                                        bounded_int<unsigned, 0, 2>>::value);

    template <class Int, Int Value>
    constexpr bounded_int<Int, Value, Value> literal()
    {
        return bounded_int<Int, Value, Value>::template literal<Value>();
    }
}

#endif
