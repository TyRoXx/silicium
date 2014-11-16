#include <silicium/observable/coroutine_generator.hpp>
#include <silicium/observable/consume.hpp>
#include <silicium/observable/for_each.hpp>
#include <silicium/observable/bridge.hpp>
#include <silicium/to_unique.hpp>
#include <boost/test/unit_test.hpp>

namespace Si
{
	BOOST_AUTO_TEST_CASE(reactive_coroutine_generate)
	{
		auto co = Si::make_coroutine_generator<int>([](Si::push_context<int> &yield) -> void
		{
			yield(1);
			yield(2);
		});
		std::vector<int> generated;
		auto collector = Si::consume<int>([&generated](int element)
		{
			generated.emplace_back(element);
		});
		for (;;)
		{
			auto old_size = generated.size();
			co.async_get_one(collector);
			if (generated.size() == old_size)
			{
				break;
			}
			BOOST_REQUIRE(generated.size() == old_size + 1);
		}
		std::vector<int> const expected{1, 2};
		BOOST_CHECK(expected == generated);
	}

	BOOST_AUTO_TEST_CASE(reactive_coroutine_get_one)
	{
		Si::bridge<int> asyncs;
		bool exited_cleanly = false;
		auto co = Si::make_coroutine_generator<int>([&asyncs, &exited_cleanly](Si::push_context<int> &yield) -> void
		{
			auto a = yield.get_one(asyncs);
			BOOST_REQUIRE(a);
			yield(*a - 1);
			exited_cleanly = true;
		});
		std::vector<int> generated;
		auto collector = Si::consume<int>([&generated](int element)
		{
			generated.emplace_back(element);
		});
		co.async_get_one(collector);
		BOOST_REQUIRE(generated.empty());
		asyncs.got_element(4);

		//TODO: reading past the end should not be the required way to avoid a force unwind of the coroutine
		//      because the unwinding is done by throwing an exception.
		co.async_get_one(collector);
		BOOST_CHECK(exited_cleanly);

		std::vector<int> const expected{3};
		BOOST_CHECK(expected == generated);
	}
}

BOOST_AUTO_TEST_CASE(coroutine_generator_self_destruct)
{
	std::size_t steps_done = 0;
	auto coro = Si::to_unique(Si::make_coroutine_generator<Si::nothing>([&steps_done](Si::push_context<Si::nothing> push)
	{
		BOOST_REQUIRE_EQUAL(1u, steps_done);
		++steps_done;
		push({});
	}));
	BOOST_REQUIRE_EQUAL(0, steps_done);
	auto handler = Si::for_each(Si::ref(*coro), [&coro, &steps_done](Si::nothing)
	{
		//this function is called in the coroutine
		BOOST_REQUIRE_EQUAL(2u, steps_done);
		++steps_done;
		//destroying the coroutine itself now should not crash or anything, it just works.
		coro.reset();
	});
	BOOST_REQUIRE_EQUAL(0, steps_done);
	++steps_done;
	handler.start();
	BOOST_CHECK_EQUAL(3, steps_done);
	BOOST_CHECK(!coro);
}
