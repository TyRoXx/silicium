#include <silicium/sink/copy.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <silicium/source/range_source.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(copy_source_to_sink)
{
	std::array<int, 3> const input = {{1, 3, 4}};
	auto source = Si::make_range_source(Si::make_contiguous_range(input));
	std::vector<int> output;
	auto sink = Si::make_container_sink(output);
	Si::copy(source, sink);
	BOOST_CHECK_EQUAL_COLLECTIONS(
	    input.begin(), input.end(), output.begin(), output.end());
}
