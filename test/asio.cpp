#include <silicium/config.hpp>
#include <silicium/asio/async.hpp>
#include <silicium/asio/async_source.hpp>
#include <silicium/source/generator_source.hpp>
#include <algorithm>
#include <boost/asio/io_service.hpp>
#include <boost/test/unit_test.hpp>

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
