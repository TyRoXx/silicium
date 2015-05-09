#include <silicium/move.hpp>
#include <silicium/move_if_noexcept.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(move)
{
	auto p = Si::make_unique<int>(123);
	auto q = Si::move(p);
	BOOST_CHECK(!p);
	BOOST_REQUIRE(q);
	BOOST_CHECK_EQUAL(123, *q);
}

BOOST_AUTO_TEST_CASE(move_if_noexcept_true)
{
	auto p = Si::make_unique<int>(123);
	auto q = Si::move_if_noexcept(p);
	BOOST_CHECK(!p);
	BOOST_REQUIRE(q);
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

	non_noexcept_movable &operator = (non_noexcept_movable const &other)
	{
		state = other.state;
		return *this;
	}

	non_noexcept_movable &operator = (non_noexcept_movable &&other) BOOST_NOEXCEPT_IF(false)
	{
		state = other.state;
		other.state = 0;
		return *this;
	}
};

BOOST_AUTO_TEST_CASE(move_if_noexcept_false)
{
	non_noexcept_movable a;
	a.state = 123;
	non_noexcept_movable b = Si::move_if_noexcept(a);
	BOOST_CHECK_EQUAL(123, a.state);
	BOOST_CHECK_EQUAL(123, b.state);
}
