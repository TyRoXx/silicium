#include <silicium/file_operations.hpp>
#include <silicium/write_file.hpp>
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

BOOST_AUTO_TEST_CASE(test_copy_recursively)
{
	Si::absolute_path const temp = Si::temporary_directory(Si::throw_) / Si::relative_path("silicium-test_copy_recursively");
	Si::recreate_directories(temp, Si::throw_);
	Si::absolute_path const from = temp / Si::relative_path("from");
	Si::absolute_path const to = temp / Si::relative_path("to");
	Si::create_directories(from, Si::throw_);
	auto const expected = Si::make_c_str_range("Hello");
	Si::throw_if_error(Si::write_file((from / Si::relative_path("file.txt")).safe_c_str(), expected));
	Si::copy_recursively(from, to, nullptr, Si::throw_);
	std::ifstream file((to / Si::relative_path("file.txt")).c_str(), std::ios::binary);
	BOOST_REQUIRE(file);
	std::vector<char> const file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	BOOST_CHECK_EQUAL_COLLECTIONS(file_content.begin(), file_content.end(), expected.begin(), expected.end());
}
#endif
