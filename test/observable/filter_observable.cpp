#include <silicium/observable/filter.hpp>
#include <silicium/observable/for_each.hpp>
#include <silicium/observable/bridge.hpp>
#include <silicium/observable/ref.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(filter_true)
{
	Si::bridge<int> input;
	auto filtered = Si::make_filter_observable(Si::ref(input), [](int element)
	{
		BOOST_CHECK_EQUAL(3, element);
		return true;
	});
	bool got_element = false;
	auto all = Si::for_each(filtered, [&got_element](int element)
	{
		BOOST_REQUIRE(!got_element);
		BOOST_CHECK_EQUAL(3, element);
		got_element = true;
	});
	all.start();
	BOOST_CHECK(!got_element);
	input.got_element(3);
	BOOST_CHECK(got_element);
}

BOOST_AUTO_TEST_CASE(filter_false)
{
	Si::bridge<int> input;
	auto filtered = Si::make_filter_observable(Si::ref(input), [](int element)
	{
		BOOST_CHECK_EQUAL(3, element);
		return false;
	});
	auto all = Si::for_each(filtered, [](int element)
	{
		BOOST_FAIL("no element expected");
		boost::ignore_unused_variable_warning(element);
	});
	all.start();
	input.got_element(3);
}
