#include <silicium/file_size.hpp>
#include <silicium/open.hpp>
#include <silicium/sink/file_sink.hpp>
#include <boost/test/unit_test.hpp>

#if SILICIUM_HAS_EXCEPTIONS
#include <boost/filesystem/operations.hpp>

namespace
{
	boost::filesystem::path test_root()
	{
		return boost::filesystem::current_path();
	}
}

BOOST_AUTO_TEST_CASE(file_size_empty)
{
	auto const file = test_root() / "file_size_empty.txt";
	Si::file_handle handle = Si::overwrite_file(Si::native_path_string(file.c_str())).move_value();
	BOOST_CHECK_EQUAL(Si::make_optional<boost::uintmax_t>(0), Si::file_size(handle.handle).get());
}

BOOST_AUTO_TEST_CASE(file_size_non_empty)
{
	auto const file = test_root() / "file_size_non_empty.txt";
	Si::file_handle handle = Si::overwrite_file(Si::native_path_string(file.c_str())).move_value();
	{
		Si::file_sink sink(handle.handle);
		Si::append(sink, Si::file_sink::element_type{Si::make_c_str_range("Test")});
	}
	BOOST_CHECK_EQUAL(Si::make_optional<boost::uintmax_t>(4), Si::file_size(handle.handle).get());
}

BOOST_AUTO_TEST_CASE(file_size_error)
{
#ifdef _WIN32
	Si::native_file_descriptor stdinput = GetStdHandle(STD_INPUT_HANDLE);
	BOOST_REQUIRE_NE(Si::no_file_handle, stdinput);
#endif
	Si::error_or<Si::optional<boost::uintmax_t>> size = Si::file_size(
#ifdef _WIN32
		stdinput
#else
		STDIN_FILENO
#endif
		);
#ifdef _WIN32
	BOOST_REQUIRE(size.is_error());
	BOOST_CHECK_EQUAL(boost::system::error_code(ERROR_INVALID_HANDLE, boost::system::system_category()), size.error());
#else
	BOOST_CHECK_EQUAL(Si::none, size.get());
#endif
}

#endif
