#include <silicium/byte.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/unordered_map.hpp>
#include <unordered_map>

BOOST_AUTO_TEST_CASE(byte_zero)
{
	Si::byte b = Si::byte::zero;
	BOOST_STATIC_ASSERT(sizeof(b) == 1);
	BOOST_CHECK_EQUAL(0, static_cast<char>(b));
	BOOST_CHECK_EQUAL(0, static_cast<signed char>(b));
	BOOST_CHECK_EQUAL(0, static_cast<unsigned char>(b));
	BOOST_CHECK_EQUAL(0, *reinterpret_cast<char *>(&b));
}

BOOST_AUTO_TEST_CASE(byte_std_hash)
{
	std::unordered_map<Si::byte, int> m;
	m.insert(std::make_pair(Si::byte::one, 1));
}

BOOST_AUTO_TEST_CASE(byte_boost_hash)
{
	boost::unordered_map<Si::byte, int> m;
	m.insert(std::make_pair(Si::byte::one, 1));
}

BOOST_AUTO_TEST_CASE(byte_ostream)
{
	BOOST_CHECK_EQUAL("0", boost::lexical_cast<std::string>(Si::byte::zero));
	BOOST_CHECK_EQUAL("1", boost::lexical_cast<std::string>(Si::byte::one));
	BOOST_CHECK_EQUAL("0", boost::lexical_cast<std::string>(Si::byte::minimum));
	BOOST_CHECK_EQUAL("255", boost::lexical_cast<std::string>(Si::byte::maximum));
}
