#include <silicium/function_observable.hpp>
#include <silicium/consume.hpp>
#include <boost/optional.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(function_observable_trivial)
{
	auto o = Si::make_function_observable<int>([](Si::observer<int> &receiver)
	{
		receiver.got_element(2);
	});
	boost::optional<int> result;
	auto consumer = Si::consume<int>([&result](int r)
	{
		result = r;
	});
	o.async_get_one(consumer);
	BOOST_CHECK_EQUAL(boost::make_optional(2), result);
}
