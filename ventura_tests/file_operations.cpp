#include <ventura/file_operations.hpp>
#include <ventura/write_file.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <fstream>

#if VENTURA_HAS_ABSOLUTE_PATH_OPERATIONS

#ifdef _WIN32
#	define SILICIUM_TEST_ROOT L"C:/"
#else
#	define SILICIUM_TEST_ROOT "/"
#endif

namespace
{
	ventura::absolute_path const absolute_root = *ventura::absolute_path::create(SILICIUM_TEST_ROOT);
}

BOOST_AUTO_TEST_CASE(get_current_executable_path_throw)
{
	ventura::absolute_path p = ventura::get_current_executable_path(Si::throw_);
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
	Si::error_or<ventura::absolute_path> const p = ventura::get_current_executable_path(Si::variant_);
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
	bool exists = ventura::file_exists(absolute_root, Si::throw_);
	BOOST_CHECK(exists);
}

BOOST_AUTO_TEST_CASE(test_file_exists_true_variant)
{
	Si::error_or<bool> const exists = ventura::file_exists(absolute_root, Si::variant_);
	BOOST_CHECK(exists.get());
}

BOOST_AUTO_TEST_CASE(test_file_exists_false_throw)
{
	bool exists = ventura::file_exists(absolute_root / *ventura::path_segment::create("does-not-exist"), Si::throw_);
	BOOST_CHECK(!exists);
}

BOOST_AUTO_TEST_CASE(test_file_exists_false_variant)
{
	Si::error_or<bool> const exists = ventura::file_exists(absolute_root / *ventura::path_segment::create("does-not-exist"), Si::variant_);
	BOOST_CHECK(!exists.get());
}

BOOST_AUTO_TEST_CASE(test_temporary_directory_throw)
{
	ventura::absolute_path const p = ventura::temporary_directory(Si::throw_);
	BOOST_CHECK(!p.empty());
}

BOOST_AUTO_TEST_CASE(test_temporary_directory_variant)
{
	Si::error_or<ventura::absolute_path> const p = ventura::temporary_directory(Si::variant_);
	BOOST_REQUIRE(!p.is_error());
	BOOST_CHECK(!p.get().empty());
}

BOOST_AUTO_TEST_CASE(test_get_home)
{
	ventura::absolute_path const home = ventura::get_home();
#ifdef _WIN32
	BOOST_CHECK(boost::algorithm::starts_with(to_os_string(home), L"C:\\Users\\"));
	BOOST_CHECK(boost::algorithm::ends_with(to_os_string(home), L"\\AppData\\Local"));
#elif defined(__linux__)
	BOOST_CHECK(boost::algorithm::starts_with(ventura::to_os_string(home), "/home/"));
#else
	//OSX
	BOOST_CHECK(boost::algorithm::starts_with(to_os_string(home), "/Users/"));
#endif
}

BOOST_AUTO_TEST_CASE(test_copy_recursively)
{
	ventura::absolute_path const temp = ventura::temporary_directory(Si::throw_) / ventura::relative_path("silicium-test_copy_recursively");
	ventura::recreate_directories(temp, Si::throw_);
	ventura::absolute_path const from = temp / ventura::relative_path("from");
	ventura::absolute_path const to = temp / ventura::relative_path("to");
	ventura::create_directories(from, Si::throw_);
	auto const expected = Si::make_c_str_range("Hello");
	Si::throw_if_error(ventura::write_file((from / ventura::relative_path("file.txt")).safe_c_str(), expected));
	ventura::copy_recursively(from, to, nullptr, Si::throw_);
	std::ifstream file((to / ventura::relative_path("file.txt")).c_str(), std::ios::binary);
	BOOST_REQUIRE(file);
	std::vector<char> const file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	BOOST_CHECK_EQUAL_COLLECTIONS(file_content.begin(), file_content.end(), expected.begin(), expected.end());
}
#endif
