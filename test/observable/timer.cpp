#include <silicium/asio/timer.hpp>
#include <silicium/observable/constant.hpp>
#include <silicium/observable/total_consumer.hpp>
#include <silicium/observable/coroutine.hpp>
#include <boost/test/unit_test.hpp>
#include <memory>

#if SILICIUM_HAS_COROUTINE_OBSERVABLE
BOOST_AUTO_TEST_CASE(asio_timer)
{
	boost::asio::io_service io;
	auto t = Si::asio::make_timer(io);
	std::size_t elapsed_count = 0;
	std::size_t const loop_count = 10;
	auto coro = Si::make_total_consumer(Si::make_coroutine([&t, &elapsed_count, loop_count](Si::yield_context yield)
	                                                       {
		                                                       BOOST_REQUIRE_EQUAL(0U, elapsed_count);
		                                                       for (std::size_t i = 0; i < loop_count; ++i)
		                                                       {
			                                                       t.expires_from_now(boost::chrono::microseconds(1));
			                                                       boost::optional<Si::asio::timer_elapsed> e =
			                                                           yield.get_one(t);
			                                                       BOOST_REQUIRE(e);
			                                                       ++elapsed_count;
		                                                       }
		                                                       return Si::unit{};
		                                                   }));
	coro.start();
	io.run();
	BOOST_CHECK_EQUAL(loop_count, elapsed_count);
}
#endif

namespace
{
	struct test_observer
	{
		typedef Si::asio::timer_elapsed element_type;

		bool is_element;

		test_observer()
		    : is_element(false)
		{
		}

		void got_element(Si::asio::timer_elapsed)
		{
			BOOST_REQUIRE(!is_element);
			is_element = true;
		}

		void ended()
		{
			BOOST_FAIL("unexpected end");
		}
	};
}

BOOST_AUTO_TEST_CASE(asio_owning_timer)
{
	boost::asio::io_service io;
	auto t = Si::asio::make_timer(io);
	t.expires_from_now(boost::chrono::microseconds(1));
	auto observer = std::make_shared<test_observer>();
	t.async_get_one(Si::any_ptr_observer<std::shared_ptr<test_observer>>(observer));
	BOOST_CHECK(!observer->is_element);
	BOOST_CHECK_EQUAL(2, observer.use_count());
	io.run();
	BOOST_CHECK(observer->is_element);
	BOOST_CHECK_EQUAL(1, observer.use_count());
}
