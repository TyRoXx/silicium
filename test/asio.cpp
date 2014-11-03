#include <silicium/bridge.hpp>
#include <silicium/for_each.hpp>
#include <silicium/ref.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/test/unit_test.hpp>

namespace Si
{
	namespace asio
	{
		template <class Observable, class Callback>
		auto make_post_forwarder(boost::asio::io_service &io, Observable &&from, Callback &&handler)
		{
			typedef typename std::decay<Observable>::type clean_observable;
			return for_each(
				std::forward<Observable>(from),
				[&io, handler = std::forward<Callback>(handler)](typename clean_observable::element_type element)
			{
				io.post([element = std::move(element), handler]() mutable
				{
					handler(std::move(element));
				});
			});
		}
	}
}

BOOST_AUTO_TEST_CASE(asio_post)
{
	boost::asio::io_service io;
	Si::bridge<int> b;
	bool got_element = false;
	auto forwarder = Si::asio::make_post_forwarder(io, Si::ref(b), [&got_element](int element)
	{
		BOOST_REQUIRE(!got_element);
		got_element = true;
		BOOST_CHECK_EQUAL(3, element);
	});
	BOOST_CHECK(!got_element);
	forwarder.start();
	BOOST_CHECK(!got_element);
	b.got_element(3);
	BOOST_CHECK(!got_element);
	io.run();
	BOOST_CHECK(got_element);
}
