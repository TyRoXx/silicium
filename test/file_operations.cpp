#include <silicium/file_operations.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/predicate.hpp>

#if SILICIUM_HAS_ABSOLUTE_PATH_OPERATIONS

#ifdef _WIN32
#	define SILICIUM_TEST_ROOT L"C:/"
#else
#	define SILICIUM_TEST_ROOT "/"
#endif

namespace
{
	Si::absolute_path const absolute_root = *Si::absolute_path::create(SILICIUM_TEST_ROOT);
}

BOOST_AUTO_TEST_CASE(get_current_executable_path_throw)
{
	Si::absolute_path p = Si::get_current_executable_path(Si::throw_);
	auto const expected =
		"unit_test"
#ifdef _WIN32
		".exe"
#endif
		;
	BOOST_CHECK_EQUAL(expected, p.to_boost_path().leaf());
}

BOOST_AUTO_TEST_CASE(get_current_executable_path_variant)
{
	Si::error_or<Si::absolute_path> const p = Si::get_current_executable_path(Si::variant_);
	BOOST_REQUIRE(!p.is_error());
	auto const expected =
		"unit_test"
#ifdef _WIN32
		".exe"
#endif
		;
	BOOST_CHECK_EQUAL(expected, p.get().to_boost_path().leaf());
}

BOOST_AUTO_TEST_CASE(test_file_exists_true_throw)
{
	bool exists = Si::file_exists(absolute_root, Si::throw_);
	BOOST_CHECK(exists);
}

BOOST_AUTO_TEST_CASE(test_file_exists_true_variant)
{
	Si::error_or<bool> const exists = Si::file_exists(absolute_root, Si::variant_);
	BOOST_CHECK(exists.get());
}

BOOST_AUTO_TEST_CASE(test_file_exists_false_throw)
{
	bool exists = Si::file_exists(absolute_root / *Si::path_segment::create("does-not-exist"), Si::throw_);
	BOOST_CHECK(!exists);
}

BOOST_AUTO_TEST_CASE(test_file_exists_false_variant)
{
	Si::error_or<bool> const exists = Si::file_exists(absolute_root / *Si::path_segment::create("does-not-exist"), Si::variant_);
	BOOST_CHECK(!exists.get());
}

BOOST_AUTO_TEST_CASE(test_temporary_directory_throw)
{
	Si::absolute_path const p = Si::temporary_directory(Si::throw_);
	BOOST_CHECK(!p.empty());
}

BOOST_AUTO_TEST_CASE(test_temporary_directory_variant)
{
	Si::error_or<Si::absolute_path> const p = Si::temporary_directory(Si::variant_);
	BOOST_REQUIRE(!p.is_error());
	BOOST_CHECK(!p.get().empty());
}

BOOST_AUTO_TEST_CASE(test_get_home)
{
	Si::absolute_path const home = Si::get_home();
#ifdef _WIN32
	BOOST_CHECK(boost::algorithm::starts_with(Si::to_os_string(home), L"C:\\Users\\"));
	BOOST_CHECK(boost::algorithm::ends_with(Si::to_os_string(home), L"\\AppData\\Local"));
#elif defined(__linux__)
	BOOST_CHECK(boost::algorithm::starts_with(Si::to_os_string(home), "/home/"));
#else
	//OSX
	BOOST_CHECK(boost::algorithm::starts_with(Si::to_os_string(home), "/Users/"));
#endif
}
#endif
