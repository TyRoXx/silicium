#include <silicium/lossless_cast.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(lossless_cast_signed_to_signed)
{
	BOOST_CHECK_EQUAL(
	    -1, Si::lossless_cast<signed char>(static_cast<signed char>(-1)));
	BOOST_CHECK_EQUAL(
	    -1, Si::lossless_cast<signed short>(static_cast<signed char>(-1)));
	BOOST_CHECK_EQUAL(
	    -1, Si::lossless_cast<signed int>(static_cast<signed char>(-1)));
	BOOST_CHECK_EQUAL(
	    -1, Si::lossless_cast<signed long>(static_cast<signed char>(-1)));
	BOOST_CHECK_EQUAL(
	    -1, Si::lossless_cast<signed long long>(static_cast<signed char>(-1)));

	BOOST_CHECK_EQUAL(
	    -1, Si::lossless_cast<signed short>(static_cast<signed short>(-1)));
	BOOST_CHECK_EQUAL(
	    -1, Si::lossless_cast<signed int>(static_cast<signed short>(-1)));
	BOOST_CHECK_EQUAL(
	    -1, Si::lossless_cast<signed long>(static_cast<signed short>(-1)));
	BOOST_CHECK_EQUAL(
	    -1, Si::lossless_cast<signed long long>(static_cast<signed short>(-1)));

	BOOST_CHECK_EQUAL(
	    -1, Si::lossless_cast<signed int>(static_cast<signed int>(-1)));
	BOOST_CHECK_EQUAL(
	    -1, Si::lossless_cast<signed long>(static_cast<signed int>(-1)));
	BOOST_CHECK_EQUAL(
	    -1, Si::lossless_cast<signed long long>(static_cast<signed int>(-1)));

	BOOST_CHECK_EQUAL(
	    -1, Si::lossless_cast<signed long>(static_cast<signed long>(-1)));
	BOOST_CHECK_EQUAL(
	    -1, Si::lossless_cast<signed long long>(static_cast<signed long>(-1)));

	BOOST_CHECK_EQUAL(-1, Si::lossless_cast<signed long long>(
	                          static_cast<signed long long>(-1)));
}

BOOST_AUTO_TEST_CASE(lossless_cast_unsigned_to_unsigned)
{
	BOOST_CHECK_EQUAL(
	    1u, Si::lossless_cast<unsigned char>(static_cast<unsigned char>(1)));
	BOOST_CHECK_EQUAL(
	    1u, Si::lossless_cast<unsigned short>(static_cast<unsigned char>(1)));
	BOOST_CHECK_EQUAL(
	    1u, Si::lossless_cast<unsigned int>(static_cast<unsigned char>(1)));
	BOOST_CHECK_EQUAL(
	    1u, Si::lossless_cast<unsigned long>(static_cast<unsigned char>(1)));
	BOOST_CHECK_EQUAL(1u, Si::lossless_cast<unsigned long long>(
	                          static_cast<unsigned char>(1)));

	BOOST_CHECK_EQUAL(
	    1u, Si::lossless_cast<unsigned short>(static_cast<unsigned short>(1)));
	BOOST_CHECK_EQUAL(
	    1u, Si::lossless_cast<unsigned int>(static_cast<unsigned short>(1)));
	BOOST_CHECK_EQUAL(
	    1u, Si::lossless_cast<unsigned long>(static_cast<unsigned short>(1)));
	BOOST_CHECK_EQUAL(1u, Si::lossless_cast<unsigned long long>(
	                          static_cast<unsigned short>(1)));

	BOOST_CHECK_EQUAL(
	    1u, Si::lossless_cast<unsigned int>(static_cast<unsigned int>(1)));
	BOOST_CHECK_EQUAL(
	    1u, Si::lossless_cast<unsigned long>(static_cast<unsigned int>(1)));
	BOOST_CHECK_EQUAL(1u, Si::lossless_cast<unsigned long long>(
	                          static_cast<unsigned int>(1)));

	BOOST_CHECK_EQUAL(
	    1u, Si::lossless_cast<unsigned long>(static_cast<unsigned long>(1)));
	BOOST_CHECK_EQUAL(1u, Si::lossless_cast<unsigned long long>(
	                          static_cast<unsigned long>(1)));

	BOOST_CHECK_EQUAL(1u, Si::lossless_cast<unsigned long long>(
	                          static_cast<unsigned long long>(1)));
}
