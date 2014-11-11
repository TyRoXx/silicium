#include <silicium/iterator_range.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(iterator_range_default_ctor)
{
	Si::iterator_range<int *> r;
	BOOST_CHECK(r.empty());
	BOOST_CHECK_EQUAL(0, r.size());
	BOOST_CHECK_EQUAL(r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(iterator_range_iterator_ctor_empty)
{
	int e = 0;
	Si::iterator_range<int *> r(&e, &e);
	BOOST_CHECK(r.empty());
	BOOST_CHECK_EQUAL(0, r.size());
	BOOST_CHECK_EQUAL(r.begin(), r.end());
	BOOST_CHECK_EQUAL(&e, r.begin());
}

BOOST_AUTO_TEST_CASE(iterator_range_iterator_ctor_non_empty)
{
	int e = 0;
	Si::iterator_range<int *> r(&e, &e + 1);
	BOOST_CHECK(!r.empty());
	BOOST_CHECK_EQUAL(1, r.size());
	BOOST_CHECK_EQUAL(&e, r.begin());
	BOOST_CHECK_EQUAL(&e + 1, r.end());
}

BOOST_AUTO_TEST_CASE(iterator_range_non_member_begin_end)
{
	int e = 0;
	Si::iterator_range<int *> r(&e, &e + 1);
	BOOST_CHECK_EQUAL(&e, begin(r));
	BOOST_CHECK_EQUAL(&e + 1, end(r));
}
