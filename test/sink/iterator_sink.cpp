#include <silicium/sink/iterator_sink.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(iterator_sink_append)
{
	std::array<char, 10> buffer = {0};
	auto sink = Si::make_iterator_sink<char>(buffer.begin());
	BOOST_CHECK_EQUAL(buffer.begin(), sink.position());
	Si::append(sink, 'A');
	BOOST_CHECK_EQUAL(buffer.begin() + 1, sink.position());
	Si::append(sink, 'B');
	BOOST_CHECK_EQUAL(buffer.begin() + 2, sink.position());
	std::array<char, 10> expected = {'A', 'B', 0};
	BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), buffer.begin(), buffer.end());
}
