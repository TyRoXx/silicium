#include <silicium/flatten.hpp>
#include <silicium/bridge.hpp>
#include <silicium/ref.hpp>
#include <silicium/for_each.hpp>
#include <silicium/coroutine.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/interprocess/sync/null_mutex.hpp>

BOOST_AUTO_TEST_CASE(flatten_trivial)
{
	Si::bridge<int> a, b, c;
	auto all = Si::flatten<boost::interprocess::null_mutex>(Si::make_coroutine<Si::shared_observable<int>>([&](Si::push_context<Si::shared_observable<int>> &yield)
	{
		yield(Si::erase_shared(Si::ref(a)));
		yield(Si::erase_shared(Si::ref(b)));
		yield(Si::erase_shared(Si::ref(c)));
	}));
	std::vector<int> generated;
	auto consumed = Si::for_each(Si::ref(all), [&generated](int element)
	{
		generated.emplace_back(element);
	});
	consumed.start();

	a.got_element(2);
	c.got_element(3);
	c.ended();
	b.got_element(4);
	a.got_element(5);

	std::vector<int> const expected{2, 3, 4, 5};
	BOOST_CHECK(expected == generated);
}
