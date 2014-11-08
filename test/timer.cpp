#include <silicium/asio/timer.hpp>
#include <silicium/constant_observable.hpp>
#include <silicium/total_consumer.hpp>
#include <silicium/coroutine.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(asio_timer)
{
	boost::asio::io_service io;
	auto t = Si::asio::make_timer(io, Si::make_constant_observable(std::chrono::microseconds(1)));
	std::size_t elapsed_count = 0;
	std::size_t const loop_count = 10;
	auto coro = Si::make_total_consumer(Si::make_coroutine([&t, &elapsed_count](Si::yield_context yield)
	{
		BOOST_REQUIRE_EQUAL(0U, elapsed_count);
		for (std::size_t i = 0; i < loop_count; ++i)
		{
			boost::optional<Si::asio::timer_elapsed> e = yield.get_one(t);
			BOOST_REQUIRE(e);
			++elapsed_count;
		}
		return Si::nothing{};
	}));
	coro.start();
	io.run();
	BOOST_CHECK_EQUAL(loop_count, elapsed_count);
}
