#include <silicium/observable/bridge.hpp>
#include <silicium/observable/for_each.hpp>
#include <silicium/config.hpp>
#include <silicium/observable/ref.hpp>
#include <silicium/asio/post_forwarder.hpp>
#include <silicium/asio/tcp_acceptor.hpp>
#include <algorithm>
#include <boost/asio/io_service.hpp>
#include <boost/test/unit_test.hpp>

#if !defined(_MSC_VER) && SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE

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

#endif

BOOST_AUTO_TEST_CASE(asio_make_tcp_acceptor)
{
	// make sure that all overloads still compile
	boost::asio::io_service io;
#if BOOST_VERSION >= 105400
	auto a = Si::asio::make_tcp_acceptor(boost::asio::ip::tcp::acceptor(io));
#endif
	auto b = Si::asio::make_tcp_acceptor(io, boost::asio::ip::tcp::endpoint());
	auto c = Si::asio::make_tcp_acceptor(Si::make_unique<boost::asio::ip::tcp::acceptor>(io));
}

namespace Si
{
	template <class ForegroundDispatcher, class BackgroundDispatcher, class NullaryFunction, class ResultHandler>
	auto async(ForegroundDispatcher &foreground, BackgroundDispatcher &background, NullaryFunction &&work,
	           ResultHandler &&handle_result)
	{
		typedef decltype(work()) result_type;
		typename boost::asio::handler_type<ResultHandler, void(result_type)>::type real_handler(
		    std::forward<ResultHandler>(handle_result));
		typename boost::asio::async_result<decltype(real_handler)> result(real_handler);
		background.post([
			SILICIUM_CAPTURE_EXPRESSION(work, std::forward<NullaryFunction>(work)),
			SILICIUM_CAPTURE_EXPRESSION(real_handler, std::move(real_handler)),
			&foreground
		]() mutable
		                {
			                // unfortunately a post()ed function must be copyable currently, therefore the shared_ptr
			                std::shared_ptr<result_type> result = Si::to_unique(work());
			                foreground.post([
				                SILICIUM_CAPTURE_EXPRESSION(result, std::move(result)),
				                SILICIUM_CAPTURE_EXPRESSION(real_handler, std::move(real_handler))
				            ]() mutable
			                                {
				                                real_handler(std::move(*result));
				                            });
			            });
		return result.get();
	}
}

BOOST_AUTO_TEST_CASE(asio_async)
{
	boost::asio::io_service background;
	boost::asio::io_service foreground;
	bool ok = false;
	Si::async(foreground, background,
	          []
	          {
		          return Si::to_unique(42);
		      },
	          [&ok](std::unique_ptr<int> result)
	          {
		          BOOST_REQUIRE(!ok);
		          BOOST_REQUIRE(result);
		          BOOST_REQUIRE_EQUAL(42, *result);
		          ok = true;
		      });
	BOOST_REQUIRE(!ok);
	foreground.run();
	foreground.reset();
	BOOST_REQUIRE(!ok);
	background.run();
	BOOST_REQUIRE(!ok);
	foreground.run();
	BOOST_CHECK(ok);
}
