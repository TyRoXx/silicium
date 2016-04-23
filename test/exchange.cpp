#include <silicium/exchange.hpp>
#include <silicium/make_unique.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(exchange_noexcept_true)
{
    auto p = Si::make_unique<int>(123);
    auto r = Si::make_unique<int>(456);
    auto q = Si::exchange(p, std::move(r));
    BOOST_CHECK(!r);
    BOOST_REQUIRE(p);
    BOOST_REQUIRE(q);
    BOOST_CHECK_EQUAL(456, *p);
    BOOST_CHECK_EQUAL(123, *q);
}

struct non_noexcept_movable
{
    int state;

    non_noexcept_movable()
        : state(0)
    {
    }

    non_noexcept_movable(non_noexcept_movable const &other)
        : state(other.state)
    {
    }

    non_noexcept_movable(non_noexcept_movable &&other) BOOST_NOEXCEPT_IF(false)
        : state(other.state)
    {
        other.state = 0;
    }

    non_noexcept_movable &operator=(non_noexcept_movable const &other)
    {
        state = other.state;
        return *this;
    }

    non_noexcept_movable &operator=(non_noexcept_movable &&other)
        BOOST_NOEXCEPT_IF(false)
    {
        state = other.state;
        other.state = 0;
        return *this;
    }
};

BOOST_AUTO_TEST_CASE(exchange_noexcept_false)
{
    non_noexcept_movable a;
    a.state = 123;
    non_noexcept_movable b;
    b.state = 456;
    non_noexcept_movable c = Si::exchange(a, std::move(b));
    BOOST_CHECK_EQUAL(456, a.state);
    BOOST_CHECK_EQUAL(123, c.state);
    BOOST_CHECK_EQUAL(0, b.state);
}
