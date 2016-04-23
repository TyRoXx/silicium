#include <silicium/observable/generator.hpp>
#include <silicium/observable/consume.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(generator_observable_optional)
{
    auto g = Si::make_generator_observable([]()
                                           {
                                               return boost::make_optional(3);
                                           });
    bool got_element = false;
    auto consumer = Si::consume<int>([&got_element](int element)
                                     {
                                         BOOST_REQUIRE(!got_element);
                                         BOOST_CHECK_EQUAL(3, element);
                                         got_element = true;
                                     });
    g.async_get_one(consumer);
    BOOST_CHECK(got_element);
}
