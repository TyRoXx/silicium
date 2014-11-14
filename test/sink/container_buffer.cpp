#include <silicium/sink/container_buffer.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(container_buffer_make)
{
	std::vector<int> v;
	Si::container_buffer<std::vector<int>> buffer = Si::make_container_buffer(v);
	BOOST_CHECK(v.empty());
}

BOOST_AUTO_TEST_CASE(container_buffer_shrink_append_space)
{
	std::vector<int> v;
	Si::container_buffer<std::vector<int>> buffer = Si::make_container_buffer(v);
	Si::iterator_range<int *> space = buffer.make_append_space(100);
	BOOST_CHECK_EQUAL(100, space.size());
	BOOST_CHECK_EQUAL(100, v.size());
	BOOST_CHECK_EQUAL(v.data(), space.begin());
	Si::iterator_range<int *> space2 = buffer.make_append_space(50);
	BOOST_CHECK_EQUAL(50, space2.size());
	BOOST_CHECK_EQUAL(50, v.size());
	BOOST_CHECK_EQUAL(space.begin(), space2.begin());
	BOOST_CHECK_EQUAL(v.data(), space2.begin());
	buffer.flush_append_space();
	BOOST_CHECK_EQUAL(50, v.size());
}

BOOST_AUTO_TEST_CASE(container_buffer_grow_append_space)
{
	std::vector<int> v;
	Si::container_buffer<std::vector<int>> buffer = Si::make_container_buffer(v);
	Si::iterator_range<int *> space = buffer.make_append_space(100);
	BOOST_CHECK_EQUAL(100, space.size());
	BOOST_CHECK_EQUAL(100, v.size());
	BOOST_CHECK_EQUAL(v.data(), space.begin());
	space = buffer.make_append_space(200);
	BOOST_CHECK_EQUAL(200, space.size());
	BOOST_CHECK_EQUAL(200, v.size());
	BOOST_CHECK_EQUAL(v.data(), space.begin());
	buffer.flush_append_space();
	BOOST_CHECK_EQUAL(200, v.size());
}

BOOST_AUTO_TEST_CASE(container_buffer_flush_append_space)
{
	std::vector<int> v;
	v.reserve(200);
	Si::container_buffer<std::vector<int>> buffer = Si::make_container_buffer(v);
	Si::iterator_range<int *> space = buffer.make_append_space(100);
	buffer.flush_append_space();
	BOOST_CHECK_EQUAL(100, v.size());
	BOOST_CHECK_EQUAL(100, space.size());
	Si::iterator_range<int *> space2 = buffer.make_append_space(100);
	BOOST_CHECK_EQUAL(200, v.size());
	BOOST_CHECK_EQUAL(100, space2.size());
	BOOST_CHECK_EQUAL(space.begin() + 100, space2.begin());
}
