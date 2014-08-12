#include <silicium/config.hpp>
#include <silicium/override.hpp>
#include <silicium/consume.hpp>
#include <silicium/virtualize.hpp>
#include <boost/test/unit_test.hpp>

namespace Si
{
	struct future_unchecked
	{
		template <class Element>
		void begin_get(Element const &)
		{
		}
	};

	struct future_runtime_checked
	{
		template <class Element>
		void begin_get(Element const &)
		{
			assert(!got);
			got = true;
		}

	private:

		bool got = false;
	};

	template <class Element, class CheckPolicy = future_unchecked>
	struct ready_future : private CheckPolicy
	{
		using element_type = Element;

		ready_future()
		{
		}

		explicit ready_future(Element value)
			: value(std::move(value))
		{
		}

		template <class ElementObserver>
		void async_get_one(ElementObserver &receiver)
		{
			CheckPolicy::begin_get(value);
			receiver.got_element(std::move(value));
		}

		void cancel()
		{
			SILICIUM_UNREACHABLE();
		}

	private:

		Element value;
	};

	template <class Element>
	auto make_ready_future(Element &&value) -> ready_future<typename std::decay<Element>::type>
	{
		return ready_future<typename std::decay<Element>::type>(std::forward<Element>(value));
	}
}

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
	auto f = Si::virtualize(Si::make_ready_future(42));

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
