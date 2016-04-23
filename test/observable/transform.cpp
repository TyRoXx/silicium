#include <silicium/observable/transform.hpp>
#include <silicium/observable/generator.hpp>
#include <silicium/observable/tuple.hpp>
#include <silicium/observable/consume.hpp>
#include <boost/test/unit_test.hpp>

#if SILICIUM_RX_TUPLE_AVAILABLE
BOOST_AUTO_TEST_CASE(transform)
{
    auto twos = Si::make_generator_observable([]
                                              {
                                                  return 2;
                                              });
    auto ones = Si::make_generator_observable([]
                                              {
                                                  return 1;
                                              });
    auto both = Si::make_tuple(twos, ones);
    auto added =
        Si::transform(both, [](std::tuple<int, int> const &element)
                      {
                          return std::get<0>(element) + std::get<1>(element);
                      });
    std::vector<int> generated;
    auto consumer = Si::consume<int>([&generated](int element)
                                     {
                                         generated.emplace_back(element);
                                     });
    added.async_get_one(Si::observe_by_ref(consumer));
    std::vector<int> const expected(1, 3);
    BOOST_CHECK(expected == generated);
}
#endif

#if SILICIUM_HAS_TRANSFORM_OBSERVABLE
namespace
{
    template <class Void>
    void test_transform_void()
    {
        auto twos = Si::make_generator_observable([]
                                                  {
                                                      return 2;
                                                  });
        bool called_tranformation = false;
        auto transformed =
            Si::transform(twos, [&called_tranformation](int element) -> Void
                          {
                              BOOST_CHECK_EQUAL(2, element);
                              BOOST_REQUIRE(!called_tranformation);
                              called_tranformation = true;
                          });
        bool called_consumer = false;
        auto consumer = Si::consume<Si::unit>(
            [&called_tranformation, &called_consumer](Si::unit)
            {
                BOOST_REQUIRE(called_tranformation);
                BOOST_REQUIRE(!called_consumer);
                called_consumer = true;
            });
        BOOST_CHECK(!called_tranformation);
        BOOST_CHECK(!called_consumer);
        transformed.async_get_one(Si::observe_by_ref(consumer));
        BOOST_CHECK(called_tranformation);
        BOOST_CHECK(called_consumer);
    }
}

BOOST_AUTO_TEST_CASE(transform_void)
{
    test_transform_void<void>();
}

BOOST_AUTO_TEST_CASE(transform_void_const)
{
    test_transform_void<void const>();
}

BOOST_AUTO_TEST_CASE(transform_void_volatile)
{
    test_transform_void<void volatile>();
}

BOOST_AUTO_TEST_CASE(transform_void_const_volatile)
{
    test_transform_void<void const volatile>();
}
#endif
