#include <boost/test/unit_test.hpp>
#include <silicium/memory_range.hpp>
#include <boost/array.hpp>

BOOST_AUTO_TEST_CASE(make_memory_range_c_str_literal)
{
	boost::iterator_range<char const *> r = Si::make_memory_range("hello");
	std::array<char, 6> const expected
	{
		'h', 'e', 'l', 'l', 'o',
		//the terminating zero is expected because std::end("hello") points after \0, not at it
		'\0'
	};
	BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_c_array)
{
	using std::begin;
	using std::end;
	char expected[3] = {1, 2, 3};
	boost::iterator_range<char *> r = Si::make_memory_range(expected);
	BOOST_CHECK_EQUAL_COLLECTIONS(begin(expected), end(expected), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_const_c_array)
{
	using std::begin;
	using std::end;
	char const expected[3] = {1, 2, 3};
	boost::iterator_range<char const *> r = Si::make_memory_range(expected);
	BOOST_CHECK_EQUAL_COLLECTIONS(begin(expected), end(expected), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_std_array)
{
	std::array<char, 3> expected{{1, 2, 3}};
	boost::iterator_range<char *> r = Si::make_memory_range(expected);
	BOOST_CHECK_EQUAL_COLLECTIONS(begin(expected), end(expected), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_const_std_array)
{
	std::array<char, 3> const expected{{1, 2, 3}};
	boost::iterator_range<char const *> r = Si::make_memory_range(expected);
	BOOST_CHECK_EQUAL_COLLECTIONS(begin(expected), end(expected), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_boost_array)
{
	using std::begin;
	using std::end;
	boost::array<char, 3> expected{{1, 2, 3}};
	boost::iterator_range<char *> r = Si::make_memory_range(expected);
	BOOST_CHECK_EQUAL_COLLECTIONS(begin(expected), end(expected), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_const_boost_array)
{
	using std::begin;
	using std::end;
	boost::array<char, 3> const expected{{1, 2, 3}};
	boost::iterator_range<char const *> r = Si::make_memory_range(expected);
	BOOST_CHECK_EQUAL_COLLECTIONS(begin(expected), end(expected), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_std_vector)
{
	std::vector<char> expected{1, 2, 3};
	boost::iterator_range<char *> r = Si::make_memory_range(expected);
	BOOST_CHECK_EQUAL_COLLECTIONS(begin(expected), end(expected), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_const_std_vector)
{
	std::vector<char> const expected{1, 2, 3};
	boost::iterator_range<char const *> r = Si::make_memory_range(expected);
	BOOST_CHECK_EQUAL_COLLECTIONS(begin(expected), end(expected), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_std_string)
{
	std::string expected = "123";
	boost::iterator_range<char *> r = Si::make_memory_range(expected);
	BOOST_CHECK_EQUAL_COLLECTIONS(begin(expected), end(expected), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_const_std_string)
{
	std::string const expected = "123";
	boost::iterator_range<char const *> r = Si::make_memory_range(expected);
	BOOST_CHECK_EQUAL_COLLECTIONS(begin(expected), end(expected), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_from_pointers)
{
	std::vector<char> expected{1, 2, 3};
	boost::iterator_range<char *> r = Si::make_memory_range(expected.data(), expected.data() + expected.size());
	BOOST_CHECK_EQUAL_COLLECTIONS(begin(expected), end(expected), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_from_pointers_to_const)
{
	std::vector<char> const expected{1, 2, 3};
	boost::iterator_range<char const *> r = Si::make_memory_range(expected.data(), expected.data() + expected.size());
	BOOST_CHECK_EQUAL_COLLECTIONS(begin(expected), end(expected), r.begin(), r.end());
}
