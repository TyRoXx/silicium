#include <silicium/thread.hpp>
#include <silicium/std_threading.hpp>
#include <silicium/consume.hpp>
#include <silicium/config.hpp>
#include <silicium/boost_threading.hpp>
#include <silicium/to_unique.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(reactive_async_empty)
{
	auto a = Si::make_thread<int, Si::std_threading>([](Si::push_context<int> &)
	{
	});
	auto consumer = Si::consume<int>([](int element)
	{
		boost::ignore_unused_variable_warning(element);
		BOOST_FAIL("No element expected here");
	});
	a.async_get_one(consumer);
	a.wait();
}

BOOST_AUTO_TEST_CASE(reactive_async)
{
	auto a = Si::make_thread<int, Si::std_threading>([](Si::push_context<int> &yield)
	{
		yield(1);
		yield(2);
		yield(3);
	});
	std::vector<int> const expected{1, 2, 3};
	std::vector<int> produced;
	std::unique_ptr<Si::observer<int>> pusher;
	pusher = Si::to_unique(Si::consume<int>([&](int element)
	{
		produced.emplace_back(element);
		if (produced.size() == expected.size())
		{
			return;
		}
		a.async_get_one(*pusher);
	}));
	a.async_get_one(*pusher);
	a.wait();
	BOOST_CHECK(expected == produced);
}

BOOST_AUTO_TEST_CASE(reactive_async_get_one)
{
	auto a = Si::make_thread<int, Si::std_threading>([](Si::push_context<int> &yield)
	{
		auto b = Si::make_thread<int, Si::boost_threading>([](Si::push_context<int> &yield)
		{
			yield(1);
			yield(2);
			yield(3);
		});
		for (;;)
		{
			auto e = yield.get_one(b);
			if (!e)
			{
				break;
			}
			yield(std::move(*e));
		}
		b.wait();
	});
	std::vector<int> const expected{1, 2, 3};
	std::vector<int> produced;
	std::unique_ptr<Si::observer<int>> pusher;
	pusher = Si::to_unique(Si::consume<int>([&](int element)
	{
		produced.emplace_back(element);
		if (produced.size() == expected.size())
		{
			return;
		}
		a.async_get_one(*pusher);
	}));
	a.async_get_one(*pusher);
	a.wait();
	BOOST_CHECK(expected == produced);
}
