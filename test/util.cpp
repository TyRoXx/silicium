#include <silicium/to_unique.hpp>
#include <silicium/to_shared.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(to_shared_test)
{
    std::shared_ptr<int> p = Si::to_shared(2);
    BOOST_REQUIRE(p);
    BOOST_CHECK_EQUAL(2, *p);
}

BOOST_AUTO_TEST_CASE(to_unique_test)
{
    std::unique_ptr<int> p = Si::to_unique(2);
    BOOST_REQUIRE(p);
    BOOST_CHECK_EQUAL(2, *p);
}
