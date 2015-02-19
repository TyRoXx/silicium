#include <silicium/optional.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(optional_default_ctor)
{
	Si::optional<int> o;
	BOOST_CHECK(!o);
	BOOST_CHECK_EQUAL(o, o);
}

BOOST_AUTO_TEST_CASE(optional_none_ctor)
{
	Si::optional<int> o(Si::none);
	BOOST_CHECK(!o);
	BOOST_CHECK_EQUAL(o, o);
}

BOOST_AUTO_TEST_CASE(optional_value_ctor)
{
	Si::optional<int> o(3);
	BOOST_CHECK_EQUAL(o, o);
	BOOST_REQUIRE(o);
	BOOST_CHECK_EQUAL(3, *o);
	BOOST_CHECK_NE(Si::optional<int>(), o);
}

BOOST_AUTO_TEST_CASE(optional_value_assignment)
{
	Si::optional<int> o;
	o = 3;
	BOOST_CHECK_EQUAL(o, o);
	BOOST_REQUIRE(o);
	BOOST_CHECK_EQUAL(3, *o);
	BOOST_CHECK_NE(Si::optional<int>(), o);
}

BOOST_AUTO_TEST_CASE(optional_assignment)
{
	Si::optional<int> u, v;
	u = 3;
	BOOST_CHECK_NE(u, v);
	v = u;
	BOOST_CHECK_EQUAL(u, v);
	BOOST_CHECK(u);
	BOOST_CHECK(v);
}

BOOST_AUTO_TEST_CASE(optional_rvalue_ctor)
{
	Si::optional<std::unique_ptr<int>> o(Si::make_unique<int>(3));
	BOOST_CHECK(o == o);
	BOOST_REQUIRE(o);
	BOOST_REQUIRE(*o);
	BOOST_CHECK_EQUAL(3, **o);
	BOOST_CHECK(Si::optional<std::unique_ptr<int>>() != o);
}

BOOST_AUTO_TEST_CASE(optional_rvalue_assignment)
{
	Si::optional<std::unique_ptr<int>> o;
	o = Si::make_unique<int>(3);
	BOOST_CHECK(o == o);
	BOOST_REQUIRE(o);
	BOOST_REQUIRE(*o);
	BOOST_CHECK_EQUAL(3, **o);
	BOOST_CHECK(Si::optional<std::unique_ptr<int>>() != o);
}
