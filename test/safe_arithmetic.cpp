#include <silicium/safe_arithmetic.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(safe_arithmetic_add_size_t)
{
	Si::safe_number<std::size_t> a(12), b(54);
	Si::optional<Si::safe_number<std::size_t>> result = a + b;
	BOOST_REQUIRE(result);
	BOOST_CHECK_EQUAL(66, result->value);
}

BOOST_AUTO_TEST_CASE(safe_arithmetic_add_size_t_overflow)
{
	Si::safe_number<std::size_t> a((std::numeric_limits<std::size_t>::max)() - 1), b(2);
	Si::optional<Si::safe_number<std::size_t>> result = a + b;
	BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(safe_arithmetic_sub_size_t)
{
	Si::safe_number<std::size_t> a(54), b(12);
	Si::optional<Si::safe_number<std::size_t>> result = a - b;
	BOOST_REQUIRE(result);
	BOOST_CHECK_EQUAL(42, result->value);
}

BOOST_AUTO_TEST_CASE(safe_arithmetic_sub_size_t_overflow)
{
	Si::safe_number<std::size_t> a(1), b(2);
	Si::optional<Si::safe_number<std::size_t>> result = a - b;
	BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(safe_arithmetic_mul_size_t)
{
	Si::safe_number<std::size_t> a(3), b(5);
	Si::optional<Si::safe_number<std::size_t>> result = a * b;
	BOOST_REQUIRE(result);
	BOOST_CHECK_EQUAL(15, result->value);
}

BOOST_AUTO_TEST_CASE(safe_arithmetic_mul_size_t_overflow)
{
	Si::safe_number<std::size_t> a((std::numeric_limits<std::size_t>::max)() - 1), b(2);
	Si::optional<Si::safe_number<std::size_t>> result = a * b;
	BOOST_CHECK(!result);
}
