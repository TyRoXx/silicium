#include <ventura/file_size.hpp>
#include <ventura/open.hpp>
#include <ventura/sink/file_sink.hpp>
#include <silicium/sink/append.hpp>
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
	Si::file_handle handle = ventura::overwrite_file(Si::native_path_string(file.c_str())).move_value();
	BOOST_CHECK_EQUAL(Si::make_optional<boost::uint64_t>(0), ventura::file_size(handle.handle).get());
}

#if SILICIUM_HAS_FILE_SINK
BOOST_AUTO_TEST_CASE(file_size_non_empty)
{
	auto const file = test_root() / "file_size_non_empty.txt";
	Si::file_handle handle = ventura::overwrite_file(Si::native_path_string(file.c_str())).move_value();
	{
		ventura::file_sink sink(handle.handle);
		Si::append(sink, ventura::file_sink::element_type{Si::make_c_str_range("Test")});
	}
	BOOST_CHECK_EQUAL(Si::make_optional<boost::uintmax_t>(4), ventura::file_size(handle.handle).get());
}
#endif

BOOST_AUTO_TEST_CASE(file_size_error)
{
	Si::error_or<Si::optional<boost::uint64_t>> size = ventura::file_size(
#ifdef _WIN32
		INVALID_HANDLE_VALUE
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
