#include <silicium/arithmetic/add.hpp>
#include <silicium/bounded_int.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(checked_add_size_t)
{
	BOOST_CHECK_EQUAL(
	    Si::overflow_or<std::size_t>(7), Si::checked_add<std::size_t>(2, 5));
}

BOOST_AUTO_TEST_CASE(operator_overflow_or_plus_overflow_or_size_t)
{
	BOOST_CHECK_EQUAL(
	    Si::overflow_or<std::size_t>(7),
	    (Si::overflow_or<std::size_t>(2) + Si::overflow_or<std::size_t>(5)));
}

BOOST_AUTO_TEST_CASE(operator_overflow_or_plus_size_t)
{
	BOOST_CHECK_EQUAL(
	    Si::overflow_or<std::size_t>(7),
	    (Si::overflow_or<std::size_t>(2) + static_cast<std::size_t>(5)));
}

BOOST_AUTO_TEST_CASE(operator_size_t_plus_overflow_or_size_t)
{
	BOOST_CHECK_EQUAL(
	    Si::overflow_or<std::size_t>(7),
	    (static_cast<std::size_t>(2) + Si::overflow_or<std::size_t>(5)));
}

BOOST_AUTO_TEST_CASE(operator_plus_assign_overflow_or_size_t)
{
	Si::overflow_or<std::size_t> left = 2;
	left += Si::overflow_or<std::size_t>(5);
	BOOST_CHECK_EQUAL(Si::overflow_or<std::size_t>(7), left);
}

BOOST_AUTO_TEST_CASE(operator_plus_assign_size_t)
{
	Si::overflow_or<std::size_t> left = 2;
	left += static_cast<std::size_t>(5);
	BOOST_CHECK_EQUAL(Si::overflow_or<std::size_t>(7), left);
}

namespace Si
{
	template <class Int, Int Min, Int Max>
	overflow_or<bounded_int<Int, Min, Max>>
	checked_add(bounded_int<Int, Min, Max> left,
	            bounded_int<Int, Min, Max> right)
	{
		auto sum =
		    bounded_int<Int, Min, Max>::create(left.value() + right.value());
		if (!sum)
		{
			return overflow;
		}
		return *sum;
	}
}

BOOST_AUTO_TEST_CASE(checked_add_bounded_int)
{
	Si::overflow_or<Si::bounded_int<int, 1, 2>> a =
	    Si::bounded_int<int, 1, 2>::literal<1>();
	Si::overflow_or<Si::bounded_int<int, 1, 2>> b =
	    Si::bounded_int<int, 1, 2>::literal<2>();
	{
		Si::overflow_or<Si::bounded_int<int, 1, 2>> c = a + a;
		BOOST_CHECK_EQUAL(
		    (Si::bounded_int<int, 1, 2>::literal<2>()), c.value());
	}
	{
		Si::overflow_or<Si::bounded_int<int, 1, 2>> d = a + b;
		BOOST_CHECK(d.is_overflow());
	}
	{
		Si::overflow_or<Si::bounded_int<int, 1, 2>> e = b + a;
		BOOST_CHECK(e.is_overflow());
	}
	{
		Si::overflow_or<Si::bounded_int<int, 1, 2>> f = b + b;
		BOOST_CHECK(f.is_overflow());
	}
}
