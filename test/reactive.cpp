#include <reactive/buffer.hpp>
#include <reactive/variant.hpp>
#include <reactive/coroutine.hpp>
#include <reactive/generate.hpp>
#include <reactive/consume.hpp>
#include <reactive/tuple.hpp>
#include <reactive/ptr_observable.hpp>
#include <reactive/transform.hpp>
#include <reactive/bridge.hpp>
#include <reactive/take.hpp>
#include <reactive/enumerate.hpp>
#include <reactive/cache.hpp>
#include <reactive/connector.hpp>
#include <reactive/receiver.hpp>
#include <reactive/deref_optional.hpp>
#include <boost/container/string.hpp>
#include <boost/test/unit_test.hpp>

namespace Si
{
	BOOST_AUTO_TEST_CASE(reactive_take)
	{
		auto zeros = rx::generate([]{ return 0; });
		auto ones  = rx::generate([]{ return 1; });
		auto both = rx::make_tuple(zeros, ones);
		std::vector<std::tuple<int, int>> const expected(4, std::make_tuple(0, 1));
		std::vector<std::tuple<int, int>> const generated = rx::take(both, expected.size());
		BOOST_CHECK(expected == generated);
	}

	BOOST_AUTO_TEST_CASE(reactive_transform)
	{
		auto twos = rx::generate([]{ return 2; });
		auto ones  = rx::generate([]{ return 1; });
		auto both = rx::make_tuple(twos, ones);
		auto added = rx::transform(both, [](std::tuple<int, int> const &element)
		{
			return std::get<0>(element) + std::get<1>(element);
		});
		std::vector<int> const expected(4, 3);
		std::vector<int> const generated = rx::take(added, expected.size());
		BOOST_CHECK(expected == generated);
	}

	BOOST_AUTO_TEST_CASE(reactive_bridge)
	{
		auto bridge = std::make_shared<rx::bridge<int>>();
		rx::ptr_observable<int, std::shared_ptr<rx::observable<int>>> first(bridge);
		auto ones  = rx::generate([]{ return 1; });
		auto both = rx::make_tuple(first, ones);
		auto added = rx::transform(both, [](std::tuple<int, int> const &element)
		{
			return std::get<0>(element) + std::get<1>(element);
		});
		std::vector<int> generated;
		auto consumer = rx::consume<int>([&generated](int element)
		{
			generated.emplace_back(element);
		});
		BOOST_CHECK(generated.empty());

		added.async_get_one(consumer);
		BOOST_CHECK(generated.empty());

		bridge->got_element(2);
		std::vector<int> const expected(1, 3);
		BOOST_CHECK(expected == generated);
	}

	BOOST_AUTO_TEST_CASE(reactive_make_buffer)
	{
		auto bridge = std::make_shared<rx::bridge<int>>();
		rx::ptr_observable<int, std::shared_ptr<rx::observable<int>>> first{bridge};
		auto buf = rx::make_buffer(first, 2);

		std::vector<int> generated;
		auto consumer = rx::consume<int>([&generated](int element)
		{
			generated.emplace_back(element);
		});
		BOOST_CHECK(generated.empty());

		for (size_t i = 0; i < 2; ++i)
		{
			BOOST_REQUIRE(bridge->is_waiting());
			bridge->got_element(7);
		}
		BOOST_CHECK(!bridge->is_waiting());
		BOOST_CHECK(generated.empty());

		buf.async_get_one(consumer);
		std::vector<int> expected(1, 7);
		BOOST_CHECK(expected == generated);

		buf.async_get_one(consumer);
		expected.emplace_back(7);
		BOOST_CHECK(expected == generated);

		buf.async_get_one(consumer);
		BOOST_CHECK(expected == generated);
	}

	BOOST_AUTO_TEST_CASE(reactive_coroutine_generate)
	{
		auto co = rx::make_coroutine<int>([](rx::yield_context<int> &yield) -> void
		{
			yield(1);
			yield(2);
		});
		std::vector<int> generated;
		auto collector = rx::consume<int>([&generated](int element)
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
		rx::bridge<int> asyncs;
		auto co = rx::make_coroutine<int>([&asyncs](rx::yield_context<int> &yield) -> void
		{
			auto a = yield.get_one(asyncs);
			BOOST_REQUIRE(a);
			yield(*a - 1);
		});
		std::vector<int> generated;
		auto collector = rx::consume<int>([&generated](int element)
		{
			generated.emplace_back(element);
		});
		co.async_get_one(collector);
		BOOST_REQUIRE(generated.empty());
		asyncs.got_element(4);
		std::vector<int> const expected{3};
		BOOST_CHECK(expected == generated);
	}

	BOOST_AUTO_TEST_CASE(reactive_make_variant)
	{
		rx::bridge<int> first;
		rx::bridge<boost::container::string> second;
		auto variants = make_variant(rx::ref(first), rx::ref(second));

		typedef Si::fast_variant<int, boost::container::string> variant;
		std::vector<variant> produced;
		auto consumer = rx::consume<variant>([&produced](variant element)
		{
			produced.emplace_back(std::move(element));
		});

		variants.async_get_one(consumer);
		BOOST_CHECK(produced.empty());
		first.got_element(4);

		variants.async_get_one(consumer);
		BOOST_CHECK_EQUAL(1,produced.size());
		second.got_element("Hi");

		std::vector<variant> const expected
		{
			4,
			boost::container::string("Hi")
		};

		BOOST_CHECK(expected == produced);
		BOOST_CHECK(!rx::get_immediate(variants));
	}
}
