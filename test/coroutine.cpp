#include <silicium/coroutine.hpp>
#include <silicium/consume.hpp>
#include <silicium/bridge.hpp>
#include <silicium/total_consumer.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(coroutine_trivial)
{
	auto coro = Si::make_coroutine([](Si::yield_context)
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

BOOST_AUTO_TEST_CASE(coroutine_yield)
{
	Si::bridge<int> e;
	auto coro = Si::make_coroutine([&e](Si::yield_context yield)
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

BOOST_AUTO_TEST_CASE(coroutine_total_consumer)
{
	bool executed = false;
	auto consumer = Si::make_total_consumer(Si::make_coroutine([&executed](Si::yield_context)
	{
		executed = true;
		return Si::nothing();
	}));
	BOOST_CHECK(!executed);
	consumer.start();
	BOOST_CHECK(executed);
}
