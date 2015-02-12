#include <silicium/c_string.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(c_string_not_set)
{
	Si::c_string str;
	BOOST_CHECK(!str.is_set());
}

BOOST_AUTO_TEST_CASE(c_string_set)
{
	Si::c_string str("hello");
	BOOST_CHECK(str.is_set());
	BOOST_CHECK(!str.empty());
	BOOST_CHECK_EQUAL(std::string("hello"), str.c_str());
}

BOOST_AUTO_TEST_CASE(c_string_empty)
{
	Si::c_string str("");
	BOOST_CHECK(str.is_set());
	BOOST_CHECK(str.empty());
	BOOST_CHECK_EQUAL(std::string(""), str.c_str());
}
