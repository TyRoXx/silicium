#include <silicium/checked_arithmetic.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(checked_add_size_t)
{
	BOOST_CHECK_EQUAL(Si::overflow_or<std::size_t>(7), Si::checked_add<std::size_t>(2, 5));
}

BOOST_AUTO_TEST_CASE(operator_overflow_or_plus_overflow_or_size_t)
{
	BOOST_CHECK_EQUAL(Si::overflow_or<std::size_t>(7),
	                  (Si::overflow_or<std::size_t>(2) + Si::overflow_or<std::size_t>(5)));
}

BOOST_AUTO_TEST_CASE(operator_overflow_or_plus_size_t)
{
	BOOST_CHECK_EQUAL(Si::overflow_or<std::size_t>(7), (Si::overflow_or<std::size_t>(2) + static_cast<std::size_t>(5)));
}

BOOST_AUTO_TEST_CASE(operator_size_t_plus_overflow_or_size_t)
{
	BOOST_CHECK_EQUAL(Si::overflow_or<std::size_t>(7), (static_cast<std::size_t>(2) + Si::overflow_or<std::size_t>(5)));
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
