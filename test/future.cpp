#include <silicium/consume.hpp>
#include <silicium/virtualized_observable.hpp>
#include <silicium/ready_future.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(ready_future)
{
	auto f = Si::make_ready_future(42);
	bool got_value = false;
	auto observer = Si::consume<int>([&got_value](int value)
	{
		BOOST_REQUIRE(!got_value);
		got_value = true;
		BOOST_CHECK_EQUAL(42, value);
	});
	f.async_get_one(observer);
	BOOST_CHECK(got_value);
}

BOOST_AUTO_TEST_CASE(virtualize)
{
	auto f = Si::virtualize_observable(Si::make_ready_future(42));

	//a virtualized observable implements the observable interface
	Si::observable<int> &v = f;

	bool got_value = false;
	auto observer = Si::consume<int>([&got_value](int value)
	{
		BOOST_REQUIRE(!got_value);
		got_value = true;
		BOOST_CHECK_EQUAL(42, value);
	});

	//the get request is forwarded as expected
	v.async_get_one(observer);

	BOOST_CHECK(got_value);
}
