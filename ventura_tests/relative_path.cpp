#include <ventura/relative_path.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/unordered_map.hpp>
#include <unordered_map>
#include <map>

BOOST_AUTO_TEST_CASE(relative_path_empty)
{
	ventura::relative_path e;
	BOOST_CHECK_EQUAL("", e.underlying());
	BOOST_CHECK_EQUAL(boost::filesystem::path(""), e.to_boost_path());
}

BOOST_AUTO_TEST_CASE(relative_path_copy_construction)
{
	ventura::relative_path b("b");
	ventura::relative_path a(b);
	BOOST_CHECK_EQUAL(a, b);
	BOOST_CHECK_EQUAL("b", a);
	BOOST_CHECK_EQUAL("b", b);
}

BOOST_AUTO_TEST_CASE(relative_path_copy_assignment)
{
	ventura::relative_path a, b("b");
	BOOST_CHECK_NE(a, b);
	a = b;
	BOOST_CHECK_EQUAL(a, b);
	BOOST_CHECK_EQUAL("b", a);
	BOOST_CHECK_EQUAL("b", b);
}

BOOST_AUTO_TEST_CASE(relative_path_move_construction)
{
	ventura::relative_path b("b");
	ventura::relative_path a(std::move(b));
	BOOST_CHECK_NE(a, b);
	BOOST_CHECK_EQUAL("b", a);
	BOOST_CHECK_EQUAL("", b);
}

BOOST_AUTO_TEST_CASE(relative_path_move_assignment)
{
	ventura::relative_path a, b("b");
	BOOST_CHECK_NE(a, b);
	a = std::move(b);
	BOOST_CHECK_NE(a, b);
	BOOST_CHECK_EQUAL("b", a);
	BOOST_CHECK_EQUAL("", b);
}

BOOST_AUTO_TEST_CASE(relative_path_equality)
{
	ventura::relative_path a, b("b"), c("c"), c2("c");
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

BOOST_AUTO_TEST_CASE(relative_path_equality_with_other_types)
{
	ventura::relative_path a, b("b");
	BOOST_CHECK_EQUAL(a, "");
	BOOST_CHECK_EQUAL(a, boost::filesystem::path());
	BOOST_CHECK_EQUAL(b, "b");
	BOOST_CHECK_EQUAL(b, boost::filesystem::path("b"));
	BOOST_CHECK_NE(a, "x");
	BOOST_CHECK_NE(a, boost::filesystem::path("x"));
}

BOOST_AUTO_TEST_CASE(relative_path_less_than)
{
	std::map<ventura::relative_path, int> m;
	m[ventura::relative_path("a")] = 1;
	m[ventura::relative_path("b")] = 2;
	m[ventura::relative_path("c")] = 3;
	BOOST_CHECK_EQUAL(3u, m.size());
	BOOST_CHECK_EQUAL(1, m[ventura::relative_path("a")]);
	BOOST_CHECK_EQUAL(2, m[ventura::relative_path("b")]);
	BOOST_CHECK_EQUAL(3, m[ventura::relative_path("c")]);
}

BOOST_AUTO_TEST_CASE(relative_path_std_hash)
{
	std::unordered_map<ventura::relative_path, int> m;
	m[ventura::relative_path("a")] = 1;
	m[ventura::relative_path("b")] = 2;
	m[ventura::relative_path("c")] = 3;
	BOOST_CHECK_EQUAL(3u, m.size());
	BOOST_CHECK_EQUAL(1, m[ventura::relative_path("a")]);
	BOOST_CHECK_EQUAL(2, m[ventura::relative_path("b")]);
	BOOST_CHECK_EQUAL(3, m[ventura::relative_path("c")]);
}

BOOST_AUTO_TEST_CASE(relative_path_boost_hash)
{
	boost::unordered_map<ventura::relative_path, int> m;
	m[ventura::relative_path("a")] = 1;
	m[ventura::relative_path("b")] = 2;
	m[ventura::relative_path("c")] = 3;
	BOOST_CHECK_EQUAL(3u, m.size());
	BOOST_CHECK_EQUAL(1, m[ventura::relative_path("a")]);
	BOOST_CHECK_EQUAL(2, m[ventura::relative_path("b")]);
	BOOST_CHECK_EQUAL(3, m[ventura::relative_path("c")]);
}
