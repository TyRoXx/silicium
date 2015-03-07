#include <silicium/optional.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/unordered_map.hpp>
#include <unordered_map>

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

BOOST_AUTO_TEST_CASE(optional_construct_inplace)
{
	Si::optional<boost::asio::io_service> a(Si::some);
	BOOST_REQUIRE(a);
	Si::optional<boost::asio::io_service::work> b(Si::some, *a);
	BOOST_REQUIRE(b);
}

namespace
{
	template <class Map>
	void test_map(Map &m)
	{
		m.insert(std::make_pair(Si::optional<int>(1), -1L));
		m.insert(std::make_pair(Si::optional<int>(2), -4L));
		m.insert(std::make_pair(Si::optional<int>(3), -7L));
		m.insert(std::make_pair(Si::optional<int>(),  -9L));
		BOOST_CHECK_EQUAL(4u, m.size());
		BOOST_CHECK_EQUAL(-1L, m[Si::optional<int>(1)]);
		BOOST_CHECK_EQUAL(-4L, m[Si::optional<int>(2)]);
		BOOST_CHECK_EQUAL(-7L, m[Si::optional<int>(3)]);
		BOOST_CHECK_EQUAL(-9L, m[Si::optional<int>()]);
	}
}

BOOST_AUTO_TEST_CASE(optional_std_hash)
{
	std::unordered_map<Si::optional<int>, long> m;
	test_map(m);
}

BOOST_AUTO_TEST_CASE(optional_boost_hash)
{
	boost::unordered_map<Si::optional<int>, long> m;
	test_map(m);
}
