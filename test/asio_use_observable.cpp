#include <silicium/asio/use_observable.hpp>
#include <silicium/observable/spawn_coroutine.hpp>
#include <silicium/error_or.hpp>
#include <algorithm>
#include <boost/asio/basic_waitable_timer.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(asio_use_observable_with_timer)
{
	boost::asio::io_service io;
	boost::asio::basic_waitable_timer<boost::chrono::steady_clock> timer(io);
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

BOOST_AUTO_TEST_CASE(asio_use_observable_with_socket)
{
	boost::asio::io_service io;
	boost::asio::ip::udp::socket socket(io);
	auto message = boost::asio::buffer("abc", 3);
	auto sent = socket.async_send_to(
		message,
		boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::loopback(), 12345),
		Si::asio::use_observable
	);
	bool got_result = false;
	Si::spawn_coroutine([&sent, &got_result, message](Si::spawn_context yield)
	{
		Si::optional<Si::error_or<std::size_t>> result = yield.get_one(sent);
		BOOST_REQUIRE(result);
		BOOST_REQUIRE(!got_result);
		BOOST_CHECK(result->is_error() || (result->get() == boost::asio::buffer_size(message)));
		got_result = true;
	});
	io.run();
	BOOST_CHECK(got_result);
}
