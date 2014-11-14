#include <silicium/sink/iterator_sink.hpp>
#include <silicium/sink/buffering_sink.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(buffering_sink_append)
{
	std::vector<int> v;
	auto buffer = Si::make_buffering_sink(Si::make_container_sink(v));
	BOOST_CHECK(v.empty());
	Si::append(buffer, 3);
	BOOST_CHECK(v.empty());
	buffer.flush();
	BOOST_CHECK(!v.empty());
	std::vector<int> const expected{3};
	BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), v.begin(), v.end());
}

BOOST_AUTO_TEST_CASE(buffering_sink_make_append_space)
{
	std::vector<int> v;
	auto buffer = Si::make_buffering_sink(Si::make_container_sink(v));
	BOOST_CHECK(v.empty());
	Si::iterator_range<int *> space = buffer.make_append_space(1);
	BOOST_REQUIRE_GE(space.size(), 1u);
	space[0] = 3;
	BOOST_CHECK(v.empty());
	buffer.flush_append_space();
	BOOST_CHECK(!v.empty());
	std::vector<int> const expected{3};
	BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), v.begin(), v.end());
}
