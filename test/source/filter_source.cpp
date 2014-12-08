#include <silicium/source/filter_source.hpp>
#include <silicium/source/generator_source.hpp>
#include <silicium/source/memory_source.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(filter_source_true)
{
	auto f = Si::make_filter_source(Si::make_generator_source([]() -> Si::optional<int> { return 2; }), [](int element)
	{
		BOOST_REQUIRE_EQUAL(2, element);
		return true;
	});
	Si::optional<int> element = Si::get(f);
	BOOST_CHECK_EQUAL(2, element);
}

BOOST_AUTO_TEST_CASE(filter_source_false)
{
	std::vector<int> const elements{1, 2, 3};
	auto f = Si::make_filter_source(Si::make_container_source(elements), [](int element)
	{
		return element >= 3;
	});
	Si::optional<int> element = Si::get(f);
	BOOST_CHECK_EQUAL(3, element);
	auto finish = Si::get(f);
	BOOST_CHECK(!finish);
}
