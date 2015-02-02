#include <silicium/file_size.hpp>
#include <silicium/open.hpp>
#include <silicium/sink/file_sink.hpp>
#include <boost/test/unit_test.hpp>
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
	Si::file_handle handle = Si::overwrite_file(file).get();
	BOOST_CHECK_EQUAL(Si::make_optional<boost::uintmax_t>(0), Si::file_size(handle.handle).get());
}

BOOST_AUTO_TEST_CASE(file_size_non_empty)
{
	auto const file = test_root() / "file_size_non_empty.txt";
	Si::file_handle handle = Si::overwrite_file(file).get();
	{
		Si::file_sink sink(handle.handle);
		Si::append(sink, Si::file_sink::element_type{Si::make_c_str_range("Test")});
	}
	BOOST_CHECK_EQUAL(Si::make_optional<boost::uintmax_t>(4), Si::file_size(handle.handle).get());
}

BOOST_AUTO_TEST_CASE(file_size_error)
{
	Si::optional<boost::uintmax_t> size = Si::file_size(STDIN_FILENO).get();
	BOOST_CHECK_EQUAL(Si::none, size);
}
