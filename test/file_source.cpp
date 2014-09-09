#include <silicium/file_source.hpp>
#include <silicium/file_descriptor.hpp>
#include <silicium/open.hpp>
#include <boost/test/unit_test.hpp>

namespace
{
}

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
	auto s = Si::make_file_source(f.get().handle, boost::make_iterator_range(buffer.data(), buffer.data() + buffer.size()));
	auto r = Si::get(s);
	BOOST_REQUIRE(r);
	BOOST_CHECK(Si::file_read_result{buffer.size()} == *r);
}