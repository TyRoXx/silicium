#include <ventura/file_operations.hpp>
#include <silicium/os_string.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/unordered_map.hpp>
#include <unordered_map>
#include <map>

#ifdef _WIN32
#	define SILICIUM_TEST_ROOT L"C:/"
#else
#	define SILICIUM_TEST_ROOT "/"
#endif

namespace
{
	Si::os_string const absolute_root(SILICIUM_TEST_ROOT);
}

BOOST_AUTO_TEST_CASE(absolute_path_empty)
{
	ventura::absolute_path e;
	BOOST_CHECK_EQUAL("", e.underlying());
	BOOST_CHECK_EQUAL(boost::filesystem::path(""), e.to_boost_path());
}

BOOST_AUTO_TEST_CASE(absolute_path_copy_construction)
{
	ventura::absolute_path b = *ventura::absolute_path::create(absolute_root + SILICIUM_SYSTEM_LITERAL("b"));
	ventura::absolute_path a(b);
	BOOST_CHECK_EQUAL(a, b);
	BOOST_CHECK(absolute_root + SILICIUM_SYSTEM_LITERAL("b") == a);
	BOOST_CHECK(absolute_root + SILICIUM_SYSTEM_LITERAL("b") == b);
}

BOOST_AUTO_TEST_CASE(absolute_path_copy_assignment)
{
	ventura::absolute_path a, b = *ventura::absolute_path::create(absolute_root + SILICIUM_SYSTEM_LITERAL("b"));
	BOOST_CHECK_NE(a, b);
	a = b;
	BOOST_CHECK_EQUAL(a, b);
	BOOST_CHECK(absolute_root + SILICIUM_SYSTEM_LITERAL("b") == a);
	BOOST_CHECK(absolute_root + SILICIUM_SYSTEM_LITERAL("b") == b);
}

BOOST_AUTO_TEST_CASE(absolute_path_move_construction)
{
	ventura::absolute_path b = *ventura::absolute_path::create(absolute_root + SILICIUM_SYSTEM_LITERAL("b"));
	ventura::absolute_path a(std::move(b));
	BOOST_CHECK_NE(a, b);
	BOOST_CHECK(absolute_root + SILICIUM_SYSTEM_LITERAL("b") == a);
	BOOST_CHECK_EQUAL("", b);
}

BOOST_AUTO_TEST_CASE(absolute_path_move_assignment)
{
	ventura::absolute_path a, b = *ventura::absolute_path::create(absolute_root + SILICIUM_SYSTEM_LITERAL("b"));
	BOOST_CHECK_NE(a, b);
	a = std::move(b);
	BOOST_CHECK_NE(a, b);
	BOOST_CHECK(absolute_root + SILICIUM_SYSTEM_LITERAL("b") == a);
	BOOST_CHECK_EQUAL("", b);
}

BOOST_AUTO_TEST_CASE(absolute_path_equality)
{
	ventura::absolute_path a, b = *ventura::absolute_path::create(absolute_root + SILICIUM_SYSTEM_LITERAL("b")), c = *ventura::absolute_path::create(absolute_root + SILICIUM_SYSTEM_LITERAL("c")), c2 = *ventura::absolute_path::create(absolute_root + SILICIUM_SYSTEM_LITERAL("c"));
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
	ventura::absolute_path a, b = *ventura::absolute_path::create(absolute_root + SILICIUM_SYSTEM_LITERAL("b"));
	BOOST_CHECK_EQUAL(a, "");
	BOOST_CHECK_EQUAL(a, boost::filesystem::path());
	BOOST_CHECK(b == absolute_root + SILICIUM_SYSTEM_LITERAL("b"));
	BOOST_CHECK(b == boost::filesystem::path((absolute_root + SILICIUM_SYSTEM_LITERAL("b")).c_str()));
	BOOST_CHECK_NE(a, "x");
	BOOST_CHECK_NE(a, boost::filesystem::path("x"));
}

namespace
{
	template <class Map>
	void test_map(Map &m)
	{
		m[*ventura::absolute_path::create(absolute_root + SILICIUM_SYSTEM_LITERAL("a"))] = 1;
		m[*ventura::absolute_path::create(absolute_root + SILICIUM_SYSTEM_LITERAL("b"))] = 2;
		m[*ventura::absolute_path::create(absolute_root + SILICIUM_SYSTEM_LITERAL("c"))] = 3;
		BOOST_CHECK_EQUAL(3u, m.size());
		BOOST_CHECK_EQUAL(1, m[*ventura::absolute_path::create(absolute_root + SILICIUM_SYSTEM_LITERAL("a"))]);
		BOOST_CHECK_EQUAL(2, m[*ventura::absolute_path::create(absolute_root + SILICIUM_SYSTEM_LITERAL("b"))]);
		BOOST_CHECK_EQUAL(3, m[*ventura::absolute_path::create(absolute_root + SILICIUM_SYSTEM_LITERAL("c"))]);
	}
}

BOOST_AUTO_TEST_CASE(absolute_path_less_than)
{
	std::map<ventura::absolute_path, int> m;
	test_map(m);
}

BOOST_AUTO_TEST_CASE(absolute_path_std_hash)
{
	std::unordered_map<ventura::absolute_path, int> m;
	test_map(m);
}

BOOST_AUTO_TEST_CASE(absolute_path_boost_hash)
{
	boost::unordered_map<ventura::absolute_path, int> m;
	test_map(m);
}

BOOST_AUTO_TEST_CASE(absolute_path_combine)
{
	BOOST_CHECK_EQUAL(*ventura::absolute_path::create(absolute_root + SILICIUM_SYSTEM_LITERAL("a/b")), *ventura::absolute_path::create(absolute_root + SILICIUM_SYSTEM_LITERAL("a")) / ventura::relative_path("b"));
}

BOOST_AUTO_TEST_CASE(absolute_path_c_str_empty)
{
	ventura::absolute_path p;
	ventura::native_path_char const *c_str = p.c_str();
	BOOST_CHECK(std::basic_string<ventura::native_path_char>(SILICIUM_SYSTEM_LITERAL("")) == c_str);
}

BOOST_AUTO_TEST_CASE(absolute_path_c_str_non_empty)
{
	ventura::absolute_path p = *ventura::absolute_path::create(absolute_root);
	ventura::native_path_char const *c_str = p.c_str();
	BOOST_CHECK(absolute_root == c_str);
}

BOOST_AUTO_TEST_CASE(absolute_path_safe_c_str_empty)
{
	ventura::absolute_path p;
	Si::native_path_string const str = p.safe_c_str();
	BOOST_CHECK(std::basic_string<ventura::native_path_char>(SILICIUM_SYSTEM_LITERAL("")) == str.c_str());
}

BOOST_AUTO_TEST_CASE(absolute_path_safe_c_str_non_empty)
{
	ventura::absolute_path p = *ventura::absolute_path::create(absolute_root);
	Si::native_path_string const str = p.safe_c_str();
	BOOST_CHECK(absolute_root == str.c_str());
}

BOOST_AUTO_TEST_CASE(absolute_path_create_literal_ok)
{
	Si::optional<ventura::absolute_path> p = ventura::absolute_path::create(SILICIUM_TEST_ROOT);
	BOOST_REQUIRE(p);
	BOOST_CHECK_EQUAL(SILICIUM_TEST_ROOT, p->to_boost_path());
}

BOOST_AUTO_TEST_CASE(absolute_path_create_literal_not_ok)
{
	Si::optional<ventura::absolute_path> p = ventura::absolute_path::create("not root");
	BOOST_CHECK(!p);
}

#ifndef _WIN32
BOOST_AUTO_TEST_CASE(absolute_path_create_noexcept_string_ok)
{
	Si::optional<ventura::absolute_path> p = ventura::absolute_path::create(Si::noexcept_string(SILICIUM_TEST_ROOT));
	BOOST_REQUIRE(p);
	BOOST_CHECK_EQUAL(SILICIUM_TEST_ROOT, p->to_boost_path());
}

BOOST_AUTO_TEST_CASE(absolute_path_create_noexcept_string_not_ok)
{
	Si::optional<ventura::absolute_path> p = ventura::absolute_path::create(Si::noexcept_string("not root"));
	BOOST_CHECK(!p);
}
#endif

BOOST_AUTO_TEST_CASE(absolute_path_create_boost_path_ok)
{
	Si::optional<ventura::absolute_path> p = ventura::absolute_path::create(boost::filesystem::path(SILICIUM_TEST_ROOT));
	BOOST_REQUIRE(p);
	BOOST_CHECK_EQUAL(SILICIUM_TEST_ROOT, p->to_boost_path());
}

BOOST_AUTO_TEST_CASE(absolute_path_create_boost_path_not_ok)
{
	Si::optional<ventura::absolute_path> p = ventura::absolute_path::create(boost::filesystem::path("not root"));
	BOOST_CHECK(!p);
}

#ifdef _WIN32
BOOST_AUTO_TEST_CASE(absolute_path_create_wchar_not_ok)
{
	Si::optional<ventura::absolute_path> p = ventura::absolute_path::create(L"not root");
	BOOST_CHECK(!p);
}
#endif
