#include <silicium/future.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/asio/io_service.hpp>

#if SILICIUM_HAS_FUTURE

//asio::spawn should be immediately resumable since 1.58
#define SILICIUM_TEST_SPAWN ((BOOST_VERSION >= 105800) && SILICIUM_HAS_EXCEPTIONS)

#if SILICIUM_TEST_SPAWN
#include <boost/asio/spawn.hpp>
#endif

BOOST_AUTO_TEST_CASE(future_default_constructor)
{
	Si::future<int> f;
}

BOOST_AUTO_TEST_CASE(ready_future)
{
	Si::future<int> f = Si::make_ready_future(5);
	boost::asio::io_service io;
	bool got_result = false;
	f.async_wait(io.wrap([&got_result](int result)
	{
		BOOST_REQUIRE(!got_result);
		BOOST_CHECK_EQUAL(5, result);
		got_result = true;
	}));
	BOOST_CHECK(!got_result);
	io.run();
	BOOST_CHECK(got_result);
}

#if SILICIUM_TEST_SPAWN
BOOST_AUTO_TEST_CASE(future_async_wait_in_asio_spawn)
{
	Si::future<int> f = Si::make_ready_future(5);
	boost::asio::io_service io;
	bool got_result = false;
	boost::asio::spawn(io, [&got_result, &f](boost::asio::yield_context yield)
	{
		BOOST_REQUIRE(!got_result);
		int result = f.async_wait(yield);
		BOOST_CHECK_EQUAL(5, result);
		got_result = true;
	});
	BOOST_CHECK(!got_result);
	io.run();
	BOOST_CHECK(got_result);
}
#endif
#endif
