#include <silicium/vector.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(vector_default_constructor)
{
	Si::vector<int> v;
}

BOOST_AUTO_TEST_CASE(vector_move_constructor)
{
	Si::vector<int> v;
	Si::vector<int> w{std::move(v)};
}

BOOST_AUTO_TEST_CASE(vector_move_assignment)
{
	Si::vector<int> v;
	Si::vector<int> w;
	w = std::move(v);
}

BOOST_AUTO_TEST_CASE(vector_emplace_back)
{
	Si::vector<int> v;
	v.emplace_back(2);
	BOOST_CHECK_EQUAL(1, v.size());
}

BOOST_AUTO_TEST_CASE(vector_copy)
{
	Si::vector<int> v{1, 2, 3};
	Si::vector<int> w = v.copy();
	BOOST_CHECK_EQUAL(3, w.size());
	BOOST_CHECK(v == w);
}

BOOST_AUTO_TEST_CASE(vector_for)
{
	Si::vector<int> v{1, 2, 3};
	int expected = 1;
	for (int &e : v)
	{
		BOOST_CHECK_EQUAL(expected, e);
		++expected;
	}
	BOOST_CHECK_EQUAL(4, expected);
}
