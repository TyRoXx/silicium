#include <silicium/coroutine.hpp>
#include <silicium/consume.hpp>
#include <silicium/bridge.hpp>
#include <boost/test/unit_test.hpp>

namespace Si
{
	BOOST_AUTO_TEST_CASE(reactive_coroutine_trivial)
	{
		auto coro = Si::make_coroutine([](yield_context)
		{
			return 2;
		});
		bool got_element = false;
		auto consumer_ = Si::consume<int>([&got_element](int i)
		{
			BOOST_CHECK_EQUAL(2, i);
			got_element = true;
		});
		coro.async_get_one(consumer_);
		BOOST_CHECK(got_element);
	}

	BOOST_AUTO_TEST_CASE(reactive_coroutine_yield)
	{
		Si::bridge<int> e;
		auto coro = Si::make_coroutine([&e](yield_context yield)
		{
			return *yield.get_one(e) + 1;
		});
		bool got_element = false;
		auto consumer_ = Si::consume<int>([&got_element](int i)
		{
			BOOST_CHECK_EQUAL(5, i);
			got_element = true;
		});
		coro.async_get_one(consumer_);
		BOOST_CHECK(!got_element);
		e.got_element(4);
		BOOST_CHECK(got_element);
	}
}
