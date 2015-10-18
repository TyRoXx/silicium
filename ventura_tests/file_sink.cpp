#include <ventura/sink/file_sink.hpp>
#include <silicium/sink/virtualized_sink.hpp>
#include <silicium/sink/append.hpp>
#include <ventura/open.hpp>
#include <ventura/source/file_source.hpp>
#include <silicium/posix/pipe.hpp>
#include <ventura/absolute_path.hpp>
#include <silicium/throw_last_error.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread/future.hpp>
#include <array>
#include <fstream>
#if SILICIUM_HAS_EXCEPTIONS
#	include <boost/filesystem/operations.hpp>
#endif

#if SILICIUM_HAS_EXCEPTIONS && SILICIUM_HAS_FILE_SINK //for Boost filesystem
namespace
{
	std::vector<char> read_file(boost::filesystem::path const &name)
	{
		std::ifstream file(name.string(), std::ios::binary);
		return std::vector<char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	}
}

BOOST_AUTO_TEST_CASE(file_sink_success)
{
	boost::filesystem::path const file_name = boost::filesystem::temp_directory_path() / "silicium_file_sink_success.txt";
	{
		Si::file_handle file = get(ventura::overwrite_file(Si::native_path_string(file_name.c_str())));
		ventura::file_sink sink(file.handle);
		BOOST_CHECK_EQUAL(boost::system::error_code(), Si::append(sink, ventura::file_sink_element{Si::make_c_str_range("test")}));
		BOOST_CHECK_EQUAL(boost::system::error_code(), Si::append(sink, ventura::file_sink_element{ventura::flush{}}));
		BOOST_CHECK_EQUAL(boost::system::error_code(), Si::append(sink, ventura::file_sink_element{ventura::seek_set{4}}));
		BOOST_CHECK_EQUAL(boost::system::error_code(), Si::append(sink, ventura::file_sink_element{ventura::seek_add{-1}}));
		std::array<ventura::file_sink_element, 3> write_vector
		{{
			Si::make_c_str_range("aaa"),
			Si::make_c_str_range("bbbb"),
			Si::make_c_str_range("ccccc")
		}};
		BOOST_CHECK_EQUAL(boost::system::error_code(), sink.append(make_iterator_range(write_vector.data(), write_vector.data() + write_vector.size())));
	}
	std::vector<char> content = read_file(file_name);
	std::string const expected = "tesaaabbbbccccc";
	BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), content.begin(), content.end());
}
#endif

#if SILICIUM_HAS_FILE_SINK
namespace
{
	ventura::absolute_path get_readonly_file()
	{
#ifdef __linux__
		return *ventura::absolute_path::create("/proc/cpuinfo");
#endif
#ifdef _WIN32
		ventura::absolute_path file_name = *ventura::absolute_path::create(boost::filesystem::temp_directory_path()) / ventura::relative_path("silicium_file_sink_readonly.txt");
		{
			Si::error_or<Si::file_handle> file = ventura::create_file(file_name.safe_c_str());
			if (file.is_error() && (file.error() != boost::system::error_code(ERROR_FILE_EXISTS, boost::system::get_system_category())))
			{
				file.throw_if_error();
			}
		}
		if (!SetFileAttributesW(file_name.c_str(), FILE_ATTRIBUTE_READONLY))
		{
			Si::throw_last_error();
		}
		return file_name;
#endif
#ifdef BOOST_OS_MACOS
		return *ventura::absolute_path::create("/bin/ls");
#endif
	}
}

BOOST_AUTO_TEST_CASE(file_sink_error)
{
	auto file = get(ventura::open_reading(Si::native_path_string(get_readonly_file().c_str())));
	ventura::file_sink sink(file.handle);

#ifdef _WIN32
#	define SILICIUM_PLATFORM_ERROR(linux_, win32_) win32_
#else
#	define SILICIUM_PLATFORM_ERROR(linux_, win32_) linux_
#endif
	BOOST_CHECK_EQUAL(boost::system::error_code(SILICIUM_PLATFORM_ERROR(9, ERROR_ACCESS_DENIED), boost::system::system_category()), Si::append(sink, ventura::file_sink_element{Si::make_c_str_range("test")}));
	BOOST_CHECK_EQUAL(boost::system::error_code(SILICIUM_PLATFORM_ERROR(22, ERROR_ACCESS_DENIED), boost::system::system_category()), Si::append(sink, ventura::file_sink_element{ventura::flush{}}));
	BOOST_CHECK_EQUAL(boost::system::error_code(SILICIUM_PLATFORM_ERROR(22, ERROR_NEGATIVE_SEEK), boost::system::system_category()), Si::append(sink, ventura::file_sink_element{ventura::seek_add{-4}}));
	BOOST_CHECK_EQUAL(boost::system::error_code(), Si::append(sink, ventura::file_sink_element{ventura::seek_set{2}}));
#undef SILICIUM_PLATFORM_ERROR
}

#if BOOST_VERSION >= 105000 && SILICIUM_HAS_EXCEPTIONS //boost::async
BOOST_AUTO_TEST_CASE(file_sink_writev)
{
	Si::pipe buffer = Si::make_pipe().move_value();
	ventura::file_sink sink(buffer.write.handle);
	std::array<char, 9001> read_buffer;
	auto source = ventura::make_file_source(buffer.read.handle, Si::make_contiguous_range(read_buffer));
	std::vector<ventura::file_sink::element_type> writes;
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
		auto piece_buffer = piece->get();
		all_read.insert(all_read.end(), piece_buffer.begin(), piece_buffer.end());
	}

	BOOST_CHECK_EQUAL(payload.size() * 3, all_read.size());
	BOOST_CHECK(std::all_of(all_read.begin(), all_read.end(), [](char c) { return c == 'a'; }));

	writer.get();
}
#endif
#endif
