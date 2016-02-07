#include <silicium/asio/posting_observable.hpp>
#include <silicium/observable/transform.hpp>
#include <silicium/observable/ref.hpp>
#include <silicium/observable/thread.hpp>
#include <silicium/observable/total_consumer.hpp>
#include <silicium/std_threading.hpp>
#include <boost/test/unit_test.hpp>

#if BOOST_VERSION >= 105300 && SILICIUM_HAS_THREAD_OBSERVABLE && SILICIUM_HAS_TRANSFORM_OBSERVABLE
#include <boost/atomic.hpp>

BOOST_AUTO_TEST_CASE(asio_posting_observable)
{
	boost::asio::io_service io;
	boost::atomic<bool> got_result(false);
	auto posting =
	    Si::transform(Si::asio::make_posting_observable(io, Si::make_thread_observable<Si::std_threading>([]() -> int
	                                                                                                      {
		                                                                                                      return 7;
		                                                                                                  })),
	                  [&got_result](Si::optional<std::future<int>> e) -> Si::unit
	                  {
		                  BOOST_REQUIRE(e);
		                  int result = e->get();
		                  BOOST_CHECK_EQUAL(7, result);
		                  BOOST_CHECK(!got_result.load());
		                  got_result.store(true);
		                  return {};
		              });
	auto all = Si::make_total_consumer(Si::ref(posting));
	all.start();
	BOOST_CHECK(!got_result.load());
	io.run();
	BOOST_CHECK(got_result.load());
}

#endif
