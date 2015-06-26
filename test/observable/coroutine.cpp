#include <silicium/observable/coroutine.hpp>
#include <silicium/observable/consume.hpp>
#include <silicium/observable/bridge.hpp>
#include <silicium/observable/total_consumer.hpp>
#include <silicium/observable/on_first.hpp>
#include <silicium/observable/for_each.hpp>
#include <silicium/observable/spawn_coroutine.hpp>
#include <silicium/observable/limited.hpp>
#include <silicium/observable/function_observer.hpp>
#include <silicium/observable/extensible_observer.hpp>
#include <silicium/observable/spawn_observable.hpp>
#include <silicium/asio/timer.hpp>
#include <silicium/to_unique.hpp>
#include <algorithm>
#include <boost/test/unit_test.hpp>
#include <boost/asio/io_service.hpp>
#include <functional>

#if SILICIUM_HAS_COROUTINE_OBSERVABLE

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
	coro.async_get_one(Si::observe_by_ref(consumer_));
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
	coro.async_get_one(Si::observe_by_ref(consumer_));
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

BOOST_AUTO_TEST_CASE(coroutine_self_destruct)
{
	std::size_t steps_done = 0;
	auto coro = Si::to_unique(Si::make_coroutine([&steps_done](Si::yield_context) -> Si::nothing
	{
		BOOST_REQUIRE_EQUAL(1u, steps_done);
		++steps_done;
		return {};
	}));
	BOOST_REQUIRE_EQUAL(0u, steps_done);
	auto handler = Si::on_first(Si::ref(*coro), [&coro, &steps_done](boost::optional<Si::nothing> value)
	{
		BOOST_CHECK(value);
		//this function is called in the coroutine
		BOOST_REQUIRE_EQUAL(2u, steps_done);
		++steps_done;
		//destroying the coroutine itself now should not crash or anything, it just works.
		coro.reset();
	});
	BOOST_REQUIRE_EQUAL(0u, steps_done);
	++steps_done;
	handler.start();
	BOOST_CHECK_EQUAL(3u, steps_done);
	BOOST_CHECK(!coro);
}

BOOST_AUTO_TEST_CASE(spawn_coroutine_get_one)
{
	bool elapsed = false;
	boost::asio::io_service io;
	Si::spawn_coroutine([&io, &elapsed](Si::spawn_context yield)
	{
		BOOST_REQUIRE(!elapsed);
		auto timer = Si::asio::make_timer(io);
		timer.expires_from_now(boost::chrono::microseconds(1));
		Si::optional<Si::asio::timer_elapsed> e = yield.get_one(Si::ref(timer));
		BOOST_CHECK(e);
		BOOST_REQUIRE(!elapsed);
		elapsed = true;
	});
	BOOST_CHECK(!elapsed);
	io.run();
	BOOST_CHECK(elapsed);
}

BOOST_AUTO_TEST_CASE(spawn_observable)
{
	bool elapsed = false;
	boost::asio::io_service io;
	Si::spawn_observable(
		Si::transform(
			Si::make_limited_observable([&io]()
			{
				auto timer = Si::asio::make_timer(io);
				timer.expires_from_now(boost::chrono::microseconds(1));
				return timer;
			}(), 1ull),
			[&elapsed](Si::asio::timer_elapsed)
			{
				BOOST_REQUIRE(!elapsed);
				elapsed = true;
				return Si::nothing();
			}
		)
	);
	BOOST_CHECK(!elapsed);
	io.run();
	BOOST_CHECK(elapsed);
}

#endif
