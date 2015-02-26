#include <silicium/asio/use_observable.hpp>
#include <silicium/observable/spawn_coroutine.hpp>
#include <boost/asio/basic_waitable_timer.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(asio_use_observable)
{
	boost::asio::io_service io;
	boost::asio::basic_waitable_timer<boost::chrono::system_clock> timer(io);
	timer.expires_from_now(boost::chrono::nanoseconds(1));
	auto elapsed = timer.async_wait(Si::asio::use_observable);
	bool got_result = false;
	Si::spawn_coroutine([&elapsed, &got_result](Si::spawn_context yield)
	{
		Si::optional<boost::system::error_code> result = yield.get_one(elapsed);
		BOOST_REQUIRE(result);
		BOOST_CHECK(!*result);
		BOOST_REQUIRE(!got_result);
		got_result = true;
	});
	io.run();
	BOOST_CHECK(got_result);
}
