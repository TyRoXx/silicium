#include <silicium/sink/file_sink.hpp>
#include <silicium/sink/virtualized_sink.hpp>
#include <silicium/open.hpp>
#include <silicium/read_file.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(file_sink_success)
{
	boost::filesystem::path const file_name = boost::filesystem::temp_directory_path() / "silicium_file_sink_success.txt";
	{
		Si::file_handle file = get(Si::overwrite_file(file_name));
		Si::file_sink sink(file.handle);
		BOOST_CHECK_EQUAL(boost::system::error_code(), Si::append(sink, Si::file_sink_element{Si::make_c_str_range("test")}));
		BOOST_CHECK_EQUAL(boost::system::error_code(), Si::append(sink, Si::file_sink_element{Si::flush{}}));
		BOOST_CHECK_EQUAL(boost::system::error_code(), Si::append(sink, Si::file_sink_element{Si::seek_set{4}}));
		BOOST_CHECK_EQUAL(boost::system::error_code(), Si::append(sink, Si::file_sink_element{Si::seek_add{-1}}));
		std::array<Si::file_sink_element, 3> write_vector
		{{
			Si::make_c_str_range("aaa"),
			Si::make_c_str_range("bbbb"),
			Si::make_c_str_range("ccccc")
		}};
		BOOST_CHECK_EQUAL(boost::system::error_code(), sink.append(make_iterator_range(write_vector.data(), write_vector.data() + write_vector.size())));
	}
	std::vector<char> content = Si::read_file(file_name);
	std::string const expected = "tesaaabbbbccccc";
	BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), content.begin(), content.end());
}

namespace
{
	boost::filesystem::path get_readonly_file()
	{
#ifdef __linux__
		return "/proc/cpuinfo";
#endif
#ifdef _WIN32
		boost::filesystem::path file_name = boost::filesystem::temp_directory_path() / "silicium_file_sink_readonly.txt";
		{
			Si::error_or<Si::file_handle> file = Si::create_file(file_name);
			if (file.is_error() && (file.error() != boost::system::error_code(ERROR_FILE_EXISTS, boost::system::get_system_category())))
			{
				file.throw_if_error();
			}
		}
		if (!SetFileAttributesW(file_name.c_str(), FILE_ATTRIBUTE_READONLY))
		{
			boost::throw_exception(boost::system::system_error(GetLastError(), boost::system::get_system_category()));
		}
		return file_name;
#endif
	}
}

BOOST_AUTO_TEST_CASE(file_sink_error)
{
	auto file = get(Si::open_reading(get_readonly_file()));
	Si::file_sink sink(file.handle);

#ifdef _WIN32
#	define SILICIUM_PLATFORM_ERROR(linux_, win32_) win32_
#else
#	define SILICIUM_PLATFORM_ERROR(linux_, win32_) linux_
#endif
	BOOST_CHECK_EQUAL(boost::system::error_code(SILICIUM_PLATFORM_ERROR(9, ERROR_ACCESS_DENIED), boost::system::system_category()), Si::append(sink, Si::file_sink_element{Si::make_c_str_range("test")}));
	BOOST_CHECK_EQUAL(boost::system::error_code(SILICIUM_PLATFORM_ERROR(22, ERROR_ACCESS_DENIED), boost::system::system_category()), Si::append(sink, Si::file_sink_element{Si::flush{}}));
	BOOST_CHECK_EQUAL(boost::system::error_code(SILICIUM_PLATFORM_ERROR(22, ERROR_NEGATIVE_SEEK), boost::system::system_category()), Si::append(sink, Si::file_sink_element{Si::seek_add{-4}}));
	BOOST_CHECK_EQUAL(boost::system::error_code(), Si::append(sink, Si::file_sink_element{Si::seek_set{2}}));
#undef SILICIUM_PLATFORM_ERROR
}
