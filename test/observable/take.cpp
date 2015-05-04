#include <silicium/observable/take.hpp>
#include <silicium/observable/consume.hpp>
#include <silicium/observable/function.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/optional/optional_io.hpp>

BOOST_AUTO_TEST_CASE(take_observable)
{
	auto o = Si::take(Si::make_function_observable2([]()
	{
		return 2;
	}), 1);

	{
		boost::optional<int> result;
		auto consumer = Si::consume<int>([&result](int r)
		{
			BOOST_REQUIRE(!result);
			result = r;
		});
		o.async_get_one(Si::observe_by_ref(consumer));
		BOOST_CHECK_EQUAL(boost::make_optional(2), result);
	}

	{
		bool got_element = false;
		auto consumer = Si::consume<int>([&got_element](int)
		{
			got_element = true;
			BOOST_FAIL("we do not expect any further elements");
		});
		o.async_get_one(Si::observe_by_ref(consumer));
		BOOST_CHECK(!got_element);
	}
}
