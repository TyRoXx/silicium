#include <silicium/source/file_source.hpp>
#include <silicium/file_handle.hpp>
#include <silicium/open.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(file_source)
{
	auto f = Si::open_reading(
#ifdef _WIN32
		"C:/Windows/system.ini"
#else
		"/dev/zero"
#endif
		);
	std::array<char, 100> buffer;
	auto s = Si::make_file_source(f.get().handle, Si::make_iterator_range(buffer.data(), buffer.data() + buffer.size()));
	auto r = Si::get(s);
	BOOST_REQUIRE(r);
	BOOST_CHECK(Si::file_read_result{buffer.size()} == *r);
}
