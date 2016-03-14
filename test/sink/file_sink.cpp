#include <silicium/sink/file_sink.hpp>
#include <silicium/sink/append.hpp>
#include <silicium/read.hpp>
#include <silicium/pipe.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(file_sink_append)
{
	Si::pipe buffer = Si::make_pipe().move_value();
	Si::file_sink sink(buffer.write.handle);
	std::string const expected = "Hello";
	Si::append(sink, Si::make_contiguous_range(expected));
	std::array<char, 6> read_buffer;
	BOOST_REQUIRE_EQUAL(
	    5u, Si::read(buffer.read.handle, Si::make_contiguous_range(read_buffer))
	            .get());
	BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(),
	                              read_buffer.begin(),
	                              read_buffer.begin() + expected.size());
}
