#include <ventura/relative_path.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/unordered_map.hpp>
#include <unordered_map>
#include <map>

BOOST_AUTO_TEST_CASE(path_segment_empty)
{
	Si::path_segment e;
	BOOST_CHECK_EQUAL("", e.underlying());
	BOOST_CHECK_EQUAL(boost::filesystem::path(""), e.to_boost_path());
}

BOOST_AUTO_TEST_CASE(path_segment_copy_construction)
{
	Si::path_segment b = *Si::path_segment::create("b");
	Si::path_segment a(b);
	BOOST_CHECK_EQUAL(a, b);
	BOOST_CHECK_EQUAL("b", a);
	BOOST_CHECK_EQUAL("b", b);
}

BOOST_AUTO_TEST_CASE(path_segment_copy_assignment)
{
	Si::path_segment a, b = *Si::path_segment::create("b");
	BOOST_CHECK_NE(a, b);
	a = b;
	BOOST_CHECK_EQUAL(a, b);
	BOOST_CHECK_EQUAL("b", a);
	BOOST_CHECK_EQUAL("b", b);
}

BOOST_AUTO_TEST_CASE(path_segment_move_construction)
{
	Si::path_segment b = *Si::path_segment::create("b");
	Si::path_segment a(std::move(b));
	BOOST_CHECK_NE(a, b);
	BOOST_CHECK_EQUAL("b", a);
	BOOST_CHECK_EQUAL("", b);
}

BOOST_AUTO_TEST_CASE(path_segment_move_assignment)
{
	Si::path_segment a, b = *Si::path_segment::create("b");
	BOOST_CHECK_NE(a, b);
	a = std::move(b);
	BOOST_CHECK_NE(a, b);
	BOOST_CHECK_EQUAL("b", a);
	BOOST_CHECK_EQUAL("", b);
}

BOOST_AUTO_TEST_CASE(path_segment_equality)
{
	Si::path_segment a, b = *Si::path_segment::create("b"), c = *Si::path_segment::create("c"), c2 = *Si::path_segment::create("c");
	BOOST_CHECK_EQUAL(a, a);
	BOOST_CHECK_EQUAL(b, b);
	BOOST_CHECK_EQUAL(c, c);
	BOOST_CHECK_EQUAL(c, c2);
	BOOST_CHECK_EQUAL(c2, c);
	BOOST_CHECK_NE(a, b);
	BOOST_CHECK_NE(a, c);
	BOOST_CHECK_NE(b, c);
	BOOST_CHECK_NE(b, a);
	BOOST_CHECK_NE(c, b);
	BOOST_CHECK_NE(c, a);
}

BOOST_AUTO_TEST_CASE(path_segment_equality_with_other_types)
{
	Si::path_segment a, b = *Si::path_segment::create("b");
	BOOST_CHECK_EQUAL(a, "");
	BOOST_CHECK_EQUAL(a, boost::filesystem::path());
	BOOST_CHECK_EQUAL(b, "b");
	BOOST_CHECK_EQUAL(b, boost::filesystem::path("b"));
	BOOST_CHECK_NE(a, "x");
	BOOST_CHECK_NE(a, boost::filesystem::path("x"));
}

namespace
{
	template <class Map>
	void test_map(Map &m)
	{
		m[*Si::path_segment::create("a")] = 1;
		m[*Si::path_segment::create("b")] = 2;
		m[*Si::path_segment::create("c")] = 3;
		BOOST_CHECK_EQUAL(3u, m.size());
		BOOST_CHECK_EQUAL(1, m[*Si::path_segment::create("a")]);
		BOOST_CHECK_EQUAL(2, m[*Si::path_segment::create("b")]);
		BOOST_CHECK_EQUAL(3, m[*Si::path_segment::create("c")]);
	}
}

BOOST_AUTO_TEST_CASE(path_segment_less_than)
{
	std::map<Si::path_segment, int> m;
	test_map(m);
}

BOOST_AUTO_TEST_CASE(path_segment_std_hash)
{
	std::unordered_map<Si::path_segment, int> m;
	test_map(m);
}

BOOST_AUTO_TEST_CASE(path_segment_boost_hash)
{
	boost::unordered_map<Si::path_segment, int> m;
	test_map(m);
}

BOOST_AUTO_TEST_CASE(path_segment_combine)
{
	BOOST_CHECK_EQUAL(Si::relative_path("a/b"), *Si::path_segment::create("a") / *Si::path_segment::create("b"));
}
