#include <silicium/future.hpp>
#include <silicium/block_thread.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/asio/io_service.hpp>
#include <atomic>

#if SILICIUM_HAS_FUTURE

// asio::spawn should be immediately resumable since 1.58
#define SILICIUM_TEST_SPAWN                                                    \
	((BOOST_VERSION >= 105800) && SILICIUM_HAS_EXCEPTIONS &&                   \
	 !SILICIUM_AVOID_BOOST_COROUTINE)

#if SILICIUM_TEST_SPAWN
#include <boost/asio/spawn.hpp>
#endif

BOOST_AUTO_TEST_CASE(future_default_constructor)
{
	Si::future<int, Si::single_threaded> f;
	BOOST_CHECK(!f.valid());
}

BOOST_AUTO_TEST_CASE(ready_future)
{
	Si::future<int, Si::single_threaded> f =
	    Si::make_ready_future<Si::single_threaded>(5);
	BOOST_CHECK(f.valid());
	boost::asio::io_service io;
	bool got_result = false;
	f.async_get(io.wrap([&got_result](int result)
	                    {
		                    BOOST_REQUIRE(!got_result);
		                    BOOST_CHECK_EQUAL(5, result);
		                    got_result = true;
		                }));
	BOOST_CHECK(!got_result);
	io.run();
	BOOST_CHECK(got_result);
}

BOOST_AUTO_TEST_CASE(promise_early_blocking)
{
	Si::promise<int, Si::single_threaded> p;
	p.set_value(4);
	Si::future<int, Si::single_threaded> f = p.get_future();
	BOOST_CHECK(f.valid());
	BOOST_CHECK_EQUAL(4, f.async_get(Si::asio::block_thread));
	BOOST_CHECK(!f.valid());
}

BOOST_AUTO_TEST_CASE(promise_late_blocking)
{
	Si::promise<int, Si::single_threaded> p;
	Si::future<int, Si::single_threaded> f = p.get_future();
	BOOST_CHECK(f.valid());
	p.set_value(4);
	BOOST_CHECK(f.valid());
	BOOST_CHECK_EQUAL(4, f.async_get(Si::asio::block_thread));
	BOOST_CHECK(!f.valid());
}

BOOST_AUTO_TEST_CASE(promise_early_non_blocking)
{
	Si::promise<int, Si::single_threaded> p;
	p.set_value(4);
	Si::future<int, Si::single_threaded> f = p.get_future();
	BOOST_CHECK(f.valid());
	bool once = false;
	f.async_get([&once](int result)
	            {
		            BOOST_REQUIRE(!once);
		            once = true;
		            BOOST_CHECK_EQUAL(4, result);
		        });
	BOOST_CHECK(!f.valid());
	BOOST_CHECK(once);
}

BOOST_AUTO_TEST_CASE(promise_late_non_blocking)
{
	Si::promise<int, Si::single_threaded> p;
	Si::future<int, Si::single_threaded> f = p.get_future();
	BOOST_CHECK(f.valid());
	bool once = false;
	f.async_get([&once](int result)
	            {
		            BOOST_REQUIRE(!once);
		            once = true;
		            BOOST_CHECK_EQUAL(4, result);
		        });
	BOOST_CHECK(f.valid());
	BOOST_CHECK(!once);
	p.set_value(4);
	BOOST_CHECK(once);
	BOOST_CHECK(!f.valid());
}

BOOST_AUTO_TEST_CASE(promise_late_in_thread)
{
	Si::promise<int, Si::multi_threaded> p;
	Si::future<int, Si::multi_threaded> f = p.get_future();
	std::atomic<bool> once(false);
	std::future<void> background_job =
	    std::async(std::launch::async, [&once, &f]()
	               {
		               int result = f.async_get(Si::asio::block_thread);
		               BOOST_REQUIRE(!once.exchange(true));
		               BOOST_CHECK_EQUAL(23, result);
		           });
	BOOST_REQUIRE(!once.load());
	p.set_value(23);
	background_job.get();
	BOOST_CHECK(once.load());
}

#if SILICIUM_TEST_SPAWN
BOOST_AUTO_TEST_CASE(future_async_wait_in_asio_spawn)
{
	Si::future<int, Si::single_threaded> f =
	    Si::make_ready_future<Si::single_threaded>(5);
	boost::asio::io_service io;
	bool got_result = false;
	boost::asio::spawn(io, [&got_result, &f](boost::asio::yield_context yield)
	                   {
		                   BOOST_REQUIRE(!got_result);
		                   int result = f.async_get(yield);
		                   BOOST_CHECK_EQUAL(5, result);
		                   got_result = true;
		               });
	BOOST_CHECK(!got_result);
	io.run();
	BOOST_CHECK(got_result);
}
#endif
#endif
