#include <silicium/checked_arithmetic.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(checked_add_size_t)
{
	BOOST_CHECK_EQUAL(Si::overflow_or<std::size_t>(7), Si::checked_add<std::size_t>(2, 5));
}
