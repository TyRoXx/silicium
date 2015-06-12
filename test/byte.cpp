#include <silicium/byte.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(byte)
{
	Si::byte b = Si::byte::zero;
	BOOST_STATIC_ASSERT(sizeof(b) == 1);
	BOOST_CHECK_EQUAL(0, static_cast<char>(b));
	BOOST_CHECK_EQUAL(0, static_cast<signed char>(b));
	BOOST_CHECK_EQUAL(0, static_cast<unsigned char>(b));
	BOOST_CHECK_EQUAL(0, *reinterpret_cast<char *>(&b));
}
