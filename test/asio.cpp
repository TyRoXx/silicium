#include <silicium/observable/bridge.hpp>
#include <silicium/observable/for_each.hpp>
#include <silicium/config.hpp>
#include <silicium/observable/ref.hpp>
#include <silicium/asio/post_forwarder.hpp>
#include <silicium/asio/tcp_acceptor.hpp>
#include <silicium/asio/async.hpp>
#include <silicium/asio/async_source.hpp>
#include <silicium/source/generator_source.hpp>
#include <algorithm>
#include <boost/asio/io_service.hpp>
#include <boost/test/unit_test.hpp>

#if !defined(_MSC_VER) && SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE

BOOST_AUTO_TEST_CASE(asio_post)
{
    boost::asio::io_service io;
    Si::bridge<int> b;
    bool got_element = false;
    auto forwarder = Si::asio::make_post_forwarder(
        io, Si::ref(b), [&got_element](int element)
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
    auto c = Si::asio::make_tcp_acceptor(
        Si::make_unique<boost::asio::ip::tcp::acceptor>(io));
}

BOOST_AUTO_TEST_CASE(asio_async)
{
    boost::asio::io_service background;
    boost::asio::io_service foreground;
    bool ok = false;
    Si::asio::async(foreground, background,
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

#if SILICIUM_HAS_ASIO_ASYNC_SOURCE
BOOST_AUTO_TEST_CASE(asio_async_source)
{
    boost::asio::io_service background;
    auto source = Si::make_generator_source([]
                                            {
                                                return 42;
                                            });
    Si::asio::async_source<decltype(background) &, decltype(source) &> async(
        background, source);
    std::array<int, 3> buffer;
    boost::asio::io_service foreground;
    bool ok = false;
    async.async_get(
        Si::make_contiguous_range(buffer), foreground,
        [&ok,
         &buffer](Si::variant<Si::iterator_range<int const *>, int *> result)
        {
            BOOST_REQUIRE(!ok);
            Si::visit<void>(
                result,
                [](Si::iterator_range<int const *>)
                {
                    BOOST_FAIL("expected result to be copied to the buffer");
                },
                [&ok, &buffer](int *end_of_results)
                {
                    BOOST_REQUIRE_EQUAL(buffer.data() + 3, end_of_results);
                    BOOST_REQUIRE_EQUAL(42, buffer[0]);
                    BOOST_REQUIRE_EQUAL(42, buffer[1]);
                    BOOST_REQUIRE_EQUAL(42, buffer[2]);
                    ok = true;
                });
        });
    BOOST_REQUIRE(!ok);
    background.run();
    BOOST_REQUIRE(!ok);
    foreground.run();
    BOOST_CHECK(ok);
}
#endif
