#include <silicium/observable/consume.hpp>
#include <silicium/observable/virtualized.hpp>
#include <silicium/observable/ready_future.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(ready_future_observable)
{
    auto f = Si::make_ready_future_observable(42);
    bool got_value = false;
    auto observer = Si::consume<int>([&got_value](int value)
                                     {
                                         BOOST_REQUIRE(!got_value);
                                         got_value = true;
                                         BOOST_CHECK_EQUAL(42, value);
                                     });
    f.async_get_one(observer);
    BOOST_CHECK(got_value);
}

BOOST_AUTO_TEST_CASE(virtualize)
{
    auto f = Si::virtualize_observable<Si::ptr_observer<Si::observer<int>>>(
        Si::make_ready_future_observable(42));

    // a virtualized observable implements the observable interface
    Si::Observable<int, Si::ptr_observer<Si::observer<int>>>::interface &v = f;

    bool got_value = false;
    auto observer = Si::consume<int>([&got_value](int value)
                                     {
                                         BOOST_REQUIRE(!got_value);
                                         got_value = true;
                                         BOOST_CHECK_EQUAL(42, value);
                                     });

    // the get request is forwarded as expected
    v.async_get_one(Si::observe_by_ref(observer));

    BOOST_CHECK(got_value);
}
