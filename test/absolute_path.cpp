#include <silicium/absolute_path.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/unordered_map.hpp>
#include <unordered_map>
#include <map>

namespace
{
	Si::noexcept_string const absolute_root =
#ifdef _WIN32
		"C:/"
#else
		"/"
#endif
		;
}

BOOST_AUTO_TEST_CASE(absolute_path_empty)
{
	Si::absolute_path e;
	BOOST_CHECK_EQUAL("", e.underlying());
	BOOST_CHECK_EQUAL(boost::filesystem::path(""), e.to_boost_path());
}

BOOST_AUTO_TEST_CASE(absolute_path_copy_construction)
{
	Si::absolute_path b = *Si::absolute_path::create(absolute_root + "b");
	Si::absolute_path a(b);
	BOOST_CHECK_EQUAL(a, b);
	BOOST_CHECK_EQUAL(absolute_root + "b", a);
	BOOST_CHECK_EQUAL(absolute_root + "b", b);
}

BOOST_AUTO_TEST_CASE(absolute_path_copy_assignment)
{
	Si::absolute_path a, b = *Si::absolute_path::create(absolute_root + "b");
	BOOST_CHECK_NE(a, b);
	a = b;
	BOOST_CHECK_EQUAL(a, b);
	BOOST_CHECK_EQUAL(absolute_root + "b", a);
	BOOST_CHECK_EQUAL(absolute_root + "b", b);
}

BOOST_AUTO_TEST_CASE(absolute_path_move_construction)
{
	Si::absolute_path b = *Si::absolute_path::create(absolute_root + "b");
	Si::absolute_path a(std::move(b));
	BOOST_CHECK_NE(a, b);
	BOOST_CHECK_EQUAL(absolute_root + "b", a);
	BOOST_CHECK_EQUAL("", b);
}

BOOST_AUTO_TEST_CASE(absolute_path_move_assignment)
{
	Si::absolute_path a, b = *Si::absolute_path::create(absolute_root + "b");
	BOOST_CHECK_NE(a, b);
	a = std::move(b);
	BOOST_CHECK_NE(a, b);
	BOOST_CHECK_EQUAL(absolute_root + "b", a);
	BOOST_CHECK_EQUAL("", b);
}

BOOST_AUTO_TEST_CASE(absolute_path_equality)
{
	Si::absolute_path a, b = *Si::absolute_path::create(absolute_root + "b"), c = *Si::absolute_path::create(absolute_root + "c"), c2 = *Si::absolute_path::create(absolute_root + "c");
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

BOOST_AUTO_TEST_CASE(absolute_path_equality_with_other_types)
{
	Si::absolute_path a, b = *Si::absolute_path::create(absolute_root + "b");
	BOOST_CHECK_EQUAL(a, "");
	BOOST_CHECK_EQUAL(a, boost::filesystem::path());
	BOOST_CHECK_EQUAL(b, absolute_root + "b");
	BOOST_CHECK_EQUAL(b, boost::filesystem::path(absolute_root + "b"));
	BOOST_CHECK_NE(a, "x");
	BOOST_CHECK_NE(a, boost::filesystem::path("x"));
}

namespace
{
	template <class Map>
	void test_map(Map &m)
	{
		m[*Si::absolute_path::create(absolute_root + "a")] = 1;
		m[*Si::absolute_path::create(absolute_root + "b")] = 2;
		m[*Si::absolute_path::create(absolute_root + "c")] = 3;
		BOOST_CHECK_EQUAL(3u, m.size());
		BOOST_CHECK_EQUAL(1, m[*Si::absolute_path::create(absolute_root + "a")]);
		BOOST_CHECK_EQUAL(2, m[*Si::absolute_path::create(absolute_root + "b")]);
		BOOST_CHECK_EQUAL(3, m[*Si::absolute_path::create(absolute_root + "c")]);
	}
}

BOOST_AUTO_TEST_CASE(absolute_path_less_than)
{
	std::map<Si::absolute_path, int> m;
	test_map(m);
}

BOOST_AUTO_TEST_CASE(absolute_path_std_hash)
{
	std::unordered_map<Si::absolute_path, int> m;
	test_map(m);
}

BOOST_AUTO_TEST_CASE(absolute_path_boost_hash)
{
	boost::unordered_map<Si::absolute_path, int> m;
	test_map(m);
}

BOOST_AUTO_TEST_CASE(absolute_path_combine)
{
	BOOST_CHECK_EQUAL(*Si::absolute_path::create(absolute_root + "a/b"), *Si::absolute_path::create(absolute_root + "a") / Si::relative_path("b"));
}
