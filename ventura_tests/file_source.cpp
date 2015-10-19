#include <ventura/source/file_source.hpp>
#include <silicium/source/enumerating_source.hpp>
#include <silicium/source/throwing_source.hpp>
#include <silicium/file_handle.hpp>
#include <ventura/open.hpp>
#include <ventura/write_file.hpp>
#include <boost/test/unit_test.hpp>

namespace
{
	Si::file_handle read_test_file()
	{
		Si::native_path_string const name(
#ifdef _WIN32
			L"C:/Windows/system.ini"
#else
			"/dev/zero"
#endif
		);
		return ventura::open_reading(name).move_value();
	}
}

BOOST_AUTO_TEST_CASE(file_source)
{
	auto f = read_test_file();
	std::array<char, 100> buffer;
	auto s = ventura::make_file_source(f.handle, Si::make_iterator_range(buffer.data(), buffer.data() + buffer.size()));
	auto r = Si::get(s);
	BOOST_REQUIRE(r);
	BOOST_CHECK(pointing_to_same_subrange(Si::make_contiguous_range(buffer), r->get()));
}

#if VENTURA_HAS_WRITE_FILE
BOOST_AUTO_TEST_CASE(file_source_enumerate)
{
	BOOST_REQUIRE(!ventura::write_file(Si::native_path_string(SILICIUM_SYSTEM_LITERAL("test.txt")), Si::make_memory_range("Test", 4)));
	auto f = ventura::open_reading(Si::native_path_string(SILICIUM_SYSTEM_LITERAL("test.txt"))).move_value();
	std::array<char, 100> buffer;
	auto s = Si::make_enumerating_source(Si::make_throwing_source(ventura::make_file_source(f.handle, Si::make_iterator_range(buffer.data(), buffer.data() + buffer.size()))));
	BOOST_CHECK_EQUAL('T', Si::get(s));
	BOOST_CHECK_EQUAL('e', Si::get(s));
	BOOST_CHECK_EQUAL('s', Si::get(s));
	BOOST_CHECK_EQUAL('t', Si::get(s));
	BOOST_CHECK_EQUAL(Si::none, Si::get(s));
}
#endif
