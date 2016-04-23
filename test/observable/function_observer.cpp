#include <silicium/observable/function_observer.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(function_observer_got_element)
{
    bool got_element = false;
    auto o =
        Si::make_function_observer([&got_element](Si::optional<int> element)
                                   {
                                       BOOST_REQUIRE(!got_element);
                                       got_element = true;
                                       BOOST_CHECK_EQUAL(7, element);
                                   });
    BOOST_CHECK(!got_element);
    o.got_element(7);
    BOOST_CHECK(got_element);
}

BOOST_AUTO_TEST_CASE(function_observer_move)
{
    auto a = Si::make_function_observer([](Si::optional<int>)
                                        {
                                        });
    auto b = std::move(a);
    a = std::move(b);
}

BOOST_AUTO_TEST_CASE(function_observer_copy)
{
    auto a = Si::make_function_observer([](Si::optional<int>)
                                        {
                                        });
    auto b = a;
    a = b;
}

#if SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE
BOOST_AUTO_TEST_CASE(function_observer_noncopyable_function)
{
    std::unique_ptr<long> p;
    auto a =
        Si::make_function_observer([p = std::move(p)](Si::optional<int>){});
    auto b = std::move(a);
    a = std::move(b);
}
#endif
