#include <silicium/make_array.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(make_array_inferred_type)
{
	{
		std::array<int, 1> a = Si::make_array(1);
		BOOST_CHECK_EQUAL(1, a[0]);
	}
	{
		std::array<int, 2> b = Si::make_array(1, 2);
		BOOST_CHECK_EQUAL(1, b[0]);
		BOOST_CHECK_EQUAL(2, b[1]);
	}
	{
		std::array<long, 2> c = Si::make_array(1, 2L);
		BOOST_CHECK_EQUAL(1, c[0]);
		BOOST_CHECK_EQUAL(2, c[1]);
	}
	{
		std::array<long long, 3> d = Si::make_array(1, 2L, 3LL);
		BOOST_CHECK_EQUAL(1, d[0]);
		BOOST_CHECK_EQUAL(2, d[1]);
		BOOST_CHECK_EQUAL(3, d[2]);
	}
}

BOOST_AUTO_TEST_CASE(make_array_explicit_type)
{
	{
		std::array<int, 1> a = Si::make_array<int>(1);
		BOOST_CHECK_EQUAL(1, a[0]);
	}
	{
		std::array<int, 2> b = Si::make_array<int>(1, 2);
		BOOST_CHECK_EQUAL(1, b[0]);
		BOOST_CHECK_EQUAL(2, b[1]);
	}
	{
		std::array<long, 2> c = Si::make_array<long>(1, 2L);
		BOOST_CHECK_EQUAL(1, c[0]);
		BOOST_CHECK_EQUAL(2, c[1]);
	}
	{
		std::array<long long, 3> d = Si::make_array<long long>(1, 2L, 3LL);
		BOOST_CHECK_EQUAL(1, d[0]);
		BOOST_CHECK_EQUAL(2, d[1]);
		BOOST_CHECK_EQUAL(3, d[2]);
	}
	{
		std::array<long long, 1> e = Si::make_array<long long>(1);
		BOOST_CHECK_EQUAL(1, e[0]);
	}
	{
		std::array<int, 0> f = Si::make_array<int>();
		BOOST_CHECK_EQUAL(0, f.size());
	}
}
