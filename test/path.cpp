#include <silicium/path.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/unordered_map.hpp>
#include <unordered_map>
#include <map>

BOOST_AUTO_TEST_CASE(path_empty)
{
	Si::path e;
	BOOST_CHECK_EQUAL("", e.underlying());
	BOOST_CHECK_EQUAL(boost::filesystem::path(""), e.to_boost_path());
}

BOOST_AUTO_TEST_CASE(path_copy_construction)
{
	Si::path b("b");
	Si::path a(b);
	BOOST_CHECK_EQUAL(a, b);
	BOOST_CHECK_EQUAL("b", a);
	BOOST_CHECK_EQUAL("b", b);
}

BOOST_AUTO_TEST_CASE(path_copy_assignment)
{
	Si::path a, b("b");
	BOOST_CHECK_NE(a, b);
	a = b;
	BOOST_CHECK_EQUAL(a, b);
	BOOST_CHECK_EQUAL("b", a);
	BOOST_CHECK_EQUAL("b", b);
}

BOOST_AUTO_TEST_CASE(path_move_construction)
{
	Si::path b("b");
	Si::path a(std::move(b));
	BOOST_CHECK_NE(a, b);
	BOOST_CHECK_EQUAL("b", a);
	BOOST_CHECK_EQUAL("", b);
}

BOOST_AUTO_TEST_CASE(path_move_assignment)
{
	Si::path a, b("b");
	BOOST_CHECK_NE(a, b);
	a = std::move(b);
	BOOST_CHECK_NE(a, b);
	BOOST_CHECK_EQUAL("b", a);
	BOOST_CHECK_EQUAL("", b);
}

BOOST_AUTO_TEST_CASE(path_equality)
{
	Si::path a, b("b"), c("c"), c2("c");
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

BOOST_AUTO_TEST_CASE(path_equality_with_other_types)
{
	Si::path a, b("b");
	BOOST_CHECK_EQUAL(a, "");
	BOOST_CHECK_EQUAL(a, boost::filesystem::path());
	BOOST_CHECK_EQUAL(b, "b");
	BOOST_CHECK_EQUAL(b, boost::filesystem::path("b"));
	BOOST_CHECK_NE(a, "x");
	BOOST_CHECK_NE(a, boost::filesystem::path("x"));
}

BOOST_AUTO_TEST_CASE(path_less_than)
{
	std::map<Si::path, int> m;
	m[Si::path("a")] = 1;
	m[Si::path("b")] = 2;
	m[Si::path("c")] = 3;
	BOOST_CHECK_EQUAL(3, m.size());
	BOOST_CHECK_EQUAL(1, m[Si::path("a")]);
	BOOST_CHECK_EQUAL(2, m[Si::path("b")]);
	BOOST_CHECK_EQUAL(3, m[Si::path("c")]);
}

BOOST_AUTO_TEST_CASE(path_std_hash)
{
	std::unordered_map<Si::path, int> m;
	m[Si::path("a")] = 1;
	m[Si::path("b")] = 2;
	m[Si::path("c")] = 3;
	BOOST_CHECK_EQUAL(3, m.size());
	BOOST_CHECK_EQUAL(1, m[Si::path("a")]);
	BOOST_CHECK_EQUAL(2, m[Si::path("b")]);
	BOOST_CHECK_EQUAL(3, m[Si::path("c")]);
}

BOOST_AUTO_TEST_CASE(path_boost_hash)
{
	boost::unordered_map<Si::path, int> m;
	m[Si::path("a")] = 1;
	m[Si::path("b")] = 2;
	m[Si::path("c")] = 3;
	BOOST_CHECK_EQUAL(3, m.size());
	BOOST_CHECK_EQUAL(1, m[Si::path("a")]);
	BOOST_CHECK_EQUAL(2, m[Si::path("b")]);
	BOOST_CHECK_EQUAL(3, m[Si::path("c")]);
}
