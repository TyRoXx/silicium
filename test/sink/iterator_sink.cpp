#include <silicium/sink/iterator_sink.hpp>
#include <silicium/initialize_array.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(iterator_sink_append)
{
	std::array<char, 10> buffer = SILICIUM_INITIALIZE_ARRAY(0);
	auto sink = Si::make_iterator_sink<char>(buffer.begin());
	BOOST_CHECK(buffer.begin() == sink.position());
	Si::append(sink, 'A');
	BOOST_CHECK(buffer.begin() + 1 == sink.position());
	Si::append(sink, 'B');
	BOOST_CHECK(buffer.begin() + 2 == sink.position());
	std::array<char, 10> expected = SILICIUM_INITIALIZE_ARRAY('A', 'B', 0);
	BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), buffer.begin(), buffer.end());
}
