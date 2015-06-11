#include <silicium/sink/multi_sink.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <silicium/sink/function_sink.hpp>
#include <silicium/make_array.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(multi_sink_empty)
{
	auto sink = Si::make_multi_sink<int, Si::success>([]()
	{
		return Si::iterator_range<Si::Sink<int>::interface **>();
	});
	BOOST_CHECK(!Si::append(sink, 23));
}

BOOST_AUTO_TEST_CASE(multi_sink_multiple)
{
	std::vector<int> a, b;
	auto a_sink = Si::make_container_sink(a);
	auto b_sink = Si::make_container_sink(b);
	auto sinks = Si::make_array(&a_sink, &b_sink);
	auto sink = Si::make_multi_sink<int, Si::success>([sinks]()
	{
		return sinks;
	});
	BOOST_CHECK(!Si::append(sink, 23));
	BOOST_REQUIRE_EQUAL(a.size(), 1);
	BOOST_REQUIRE_EQUAL(b.size(), 1);
	BOOST_CHECK_EQUAL(a[0], 23);
	BOOST_CHECK_EQUAL(b[0], 23);
}

BOOST_AUTO_TEST_CASE(multi_sink_error)
{
	auto a = Si::Sink<int, boost::system::error_code>::make_box(Si::make_function_sink<int>([](Si::iterator_range<int const *> data)
	{
		BOOST_REQUIRE_EQUAL(data.size(), 1);
		BOOST_REQUIRE_EQUAL(data[0], 42);
		return boost::system::error_code(111, boost::system::generic_category());
	}));
	auto b = Si::Sink<int, boost::system::error_code>::make_box(Si::make_function_sink<int>([](Si::iterator_range<int const *>) -> boost::system::error_code
	{
		BOOST_FAIL("the second sink shall not be called");
		SILICIUM_UNREACHABLE();
	}));
	auto sinks = Si::make_array(a.original.get(), b.original.get());
	auto sink = Si::make_multi_sink<int, boost::system::error_code>([sinks]()
	{
		return sinks;
	});
	boost::system::error_code error = Si::append(sink, 42);
	BOOST_CHECK_EQUAL(boost::system::error_code(111, boost::system::generic_category()), error);
}
