#include <silicium/sink/file_sink.hpp>
#include <silicium/sink/virtualized_sink.hpp>
#include <silicium/open.hpp>
#include <silicium/source/file_source.hpp>
#include <silicium/read_file.hpp>
#include <silicium/posix/pipe.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread/future.hpp>
#include <array>

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

BOOST_AUTO_TEST_CASE(file_sink_writev)
{
	Si::pipe buffer = Si::make_pipe().get();
	Si::file_sink sink(buffer.write.handle);
	std::array<char, 9001> read_buffer;
	auto source = Si::make_file_source(buffer.read.handle, Si::make_contiguous_range(read_buffer));
	std::vector<Si::file_sink::element_type> writes;
	std::vector<char> const payload(0x1000000, 'a');
	writes.emplace_back(Si::make_memory_range(payload));
	writes.emplace_back(Si::make_memory_range(payload));
	writes.emplace_back(Si::make_memory_range(payload));

	auto writer = boost::async([&]()
	{
		auto error = sink.append(Si::make_contiguous_range(writes));
		BOOST_CHECK(!error);
		buffer.write.close();
	});

	std::vector<char> all_read;
	for (;;)
	{
		auto piece = Si::get(source);
		if (!piece)
		{
			break;
		}
		std::size_t size = piece->get();
		all_read.insert(all_read.end(), read_buffer.begin(), read_buffer.begin() + size);
	}

	BOOST_CHECK_EQUAL(payload.size() * 3, all_read.size());
	BOOST_CHECK(std::all_of(all_read.begin(), all_read.end(), [](char c) { return c == 'a'; }));

	writer.get();
}
