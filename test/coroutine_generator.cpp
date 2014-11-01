#include <silicium/coroutine.hpp>
#include <silicium/consume.hpp>
#include <silicium/bridge.hpp>
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
