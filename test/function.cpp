#include <silicium/function.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(function_default_constructor)
{
	Si::function<void ()> f;
	BOOST_CHECK(!f);
	Si::function<void ()> g{std::move(f)};
	BOOST_CHECK(!f);
	BOOST_CHECK(!g);
	f = std::move(g);
	BOOST_CHECK(!f);
	BOOST_CHECK(!g);
}

BOOST_AUTO_TEST_CASE(function_call)
{
	Si::function<int (int)> inc = [](int a) { return a + 1; };
	BOOST_CHECK(inc);
	BOOST_CHECK_EQUAL(3, inc(2));
	BOOST_CHECK(inc);
}
