#include <silicium/source/file_source.hpp>
#include <silicium/source/enumerating_source.hpp>
#include <silicium/source/throwing_source.hpp>
#include <silicium/file_handle.hpp>
#include <silicium/open.hpp>
#include <silicium/write_file.hpp>
#include <boost/test/unit_test.hpp>

namespace
{
	Si::file_handle read_test_file()
	{
		auto const name =
#ifdef _WIN32
			"C:/Windows/system.ini"
#else
			"/dev/zero"
#endif
		;
		return SILICIUM_MOVE_IF_COMPILER_LACKS_RVALUE_QUALIFIERS(Si::open_reading(name).get());
	}
}

BOOST_AUTO_TEST_CASE(file_source)
{
	auto f = read_test_file();
	std::array<char, 100> buffer;
	auto s = Si::make_file_source(f.handle, Si::make_iterator_range(buffer.data(), buffer.data() + buffer.size()));
	auto r = Si::get(s);
	BOOST_REQUIRE(r);
	BOOST_CHECK(pointing_to_same_subrange(Si::make_contiguous_range(buffer), r->get()));
}

BOOST_AUTO_TEST_CASE(file_source_enumerate)
{
	Si::write_file("test.txt", "Test", 4);
	auto f = SILICIUM_MOVE_IF_COMPILER_LACKS_RVALUE_QUALIFIERS(Si::open_reading("test.txt").get());
	std::array<char, 100> buffer;
	auto s = Si::make_enumerating_source(Si::make_throwing_source(Si::make_file_source(f.handle, Si::make_iterator_range(buffer.data(), buffer.data() + buffer.size()))));
	BOOST_CHECK_EQUAL('T', Si::get(s));
	BOOST_CHECK_EQUAL('e', Si::get(s));
	BOOST_CHECK_EQUAL('s', Si::get(s));
	BOOST_CHECK_EQUAL('t', Si::get(s));
	BOOST_CHECK_EQUAL(Si::none, Si::get(s));
}
