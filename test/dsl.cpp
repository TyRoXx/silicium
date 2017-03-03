#include <silicium/config.hpp>
#include <boost/test/unit_test.hpp>

namespace dsl
{
    namespace detail
    {
        template <class Type, class Name>
        struct memory_variable
        {
            Type &location;
        };

        template <class Memory, class Name>
        struct access;

        template <class Type, class Name>
        struct access<memory_variable<Type, Name>, Name>
        {
            Type &operator()(memory_variable<Type, Name> &memory) const
            {
                return memory.location;
            }
        };
    }

    template <class Variables, class EntryPoint>
    struct program
    {
        EntryPoint entry_point;

        template <class Parameter, class Argument>
        void operator()(Parameter, Argument argument) const
        {
            detail::memory_variable<Argument, Parameter> memory{argument};
            entry_point.evaluate(memory);
        }
    };

    template <class FirstExpression, class SecondExpression>
    struct sequence
    {
        FirstExpression first;
        SecondExpression second;

        template <class Memory>
        void evaluate(Memory memory) const
        {
            first.evaluate(memory);
            second.evaluate(memory);
        }
    };

    template <class Type, class Name, bool HasValue>
    struct variable
    {
    };

    namespace detail
    {
        template <class Argument>
        struct ref_expression
        {
            template <class Memory>
            auto &evaluate(Memory memory) const
            {
                return access<Memory, Argument>()(memory);
            }
        };

        template <class Argument>
        struct move_expression
        {
            template <class Memory>
            auto evaluate(Memory memory) const
            {
                return std::move(access<Memory, Argument>()(memory));
            }
        };
    }

    template <class Argument>
    auto ref(Argument)
    {
        return detail::ref_expression<Argument>();
    }

    template <class Argument>
    auto move(Argument)
    {
        return detail::move_expression<Argument>();
    }

    namespace detail
    {
        template <class Variables, class Argument>
        struct evaluate_argument;

        template <class Type, class Name, bool HasValue>
        struct evaluate_argument<variable<Type, Name, HasValue>,
                                 move_expression<Name>>
        {
            static_assert(HasValue, "Variable has already been moved from");
            using type = variable<Type, Name, false>;
        };

        template <class Type, class Name, bool HasValue>
        struct evaluate_argument<variable<Type, Name, HasValue>,
                                 ref_expression<Name>>
        {
            static_assert(HasValue, "Variable cannot be referenced because is "
                                    "has already been moved from");
            using type = variable<Type, Name, HasValue>;
        };

        template <class Argument, class F>
        struct call_expression
        {
            F f;

            template <class Memory>
            void evaluate(Memory memory) const
            {
                f(Argument().evaluate(memory));
            }
        };

        template <class Variables, class EntryPoint, class Argument, class F>
        auto operator, (program<Variables, EntryPoint> p,
                        call_expression<Argument, F> expression)
        {
            (void)expression;
            return program<
                typename evaluate_argument<Variables, Argument>::type,
                sequence<EntryPoint, decltype(expression)>>{
                {p.entry_point, std::move(expression)}};
        }
    }

    template <class F, class Argument>
    auto call(F &&f, Argument)
    {
        return detail::call_expression<Argument, F>{std::forward<F>(f)};
    }

    struct null_statement
    {
        template <class Memory>
        void evaluate(Memory &) const
        {
        }
    };

    template <class Type, class Name>
    program<variable<Type, Name, true>, null_statement> declare(Name)
    {
        return {
#ifndef _MSC_VER
            {}, {}
#endif
        };
    }
}

BOOST_AUTO_TEST_CASE(dsl_test)
{
    using namespace dsl;
    bool called_f = false;
    bool called_g = false;
    auto f = [&](std::unique_ptr<int> &value)
    {
        BOOST_REQUIRE(!called_f);
        BOOST_REQUIRE(!called_g);
        called_f = true;
        BOOST_REQUIRE_EQUAL(123, *value);
    };
    auto g = [&](std::unique_ptr<int> value)
    {
        BOOST_REQUIRE(called_f);
        BOOST_REQUIRE(!called_g);
        BOOST_REQUIRE_EQUAL(123, *value);
        called_g = true;
    };
    struct
    {
    } x;
    auto h = (declare<std::unique_ptr<int>>(x), //
              call(f, ref(x)),                  //
              call(g, move(x)));
    BOOST_REQUIRE(!called_f);
    BOOST_REQUIRE(!called_g);
    h(x, std::make_unique<int>(123));
    BOOST_REQUIRE(called_f);
    BOOST_REQUIRE(called_g);
}
