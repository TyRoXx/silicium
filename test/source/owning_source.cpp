#include <silicium/source/owning_source.hpp>
#include <boost/test/unit_test.hpp>
#include <vector>

BOOST_AUTO_TEST_CASE(owning_source_vector)
{
    std::vector<int> v = {1, 2, 3};
    Si::owning_source<std::vector<int>> s = Si::make_owning_source(v);
    BOOST_REQUIRE_EQUAL(1, Si::get(s));
    BOOST_REQUIRE_EQUAL(2, Si::get(s));
    BOOST_REQUIRE_EQUAL(3, Si::get(s));
    BOOST_REQUIRE_EQUAL(Si::none, Si::get(s));
}

BOOST_AUTO_TEST_CASE(owning_source_std_array)
{
    std::array<int, 3> a = {{1, 2, 3}};
    Si::owning_source<std::array<int, 3>> s = Si::make_owning_source(a);
    BOOST_REQUIRE_EQUAL(1, Si::get(s));
    BOOST_REQUIRE_EQUAL(2, Si::get(s));
    BOOST_REQUIRE_EQUAL(3, Si::get(s));
    BOOST_REQUIRE_EQUAL(Si::none, Si::get(s));
}

BOOST_AUTO_TEST_CASE(owning_source_boost_array)
{
    boost::array<int, 3> a = {{1, 2, 3}};
    Si::owning_source<boost::array<int, 3>> s = Si::make_owning_source(a);
    BOOST_REQUIRE_EQUAL(1, Si::get(s));
    BOOST_REQUIRE_EQUAL(2, Si::get(s));
    BOOST_REQUIRE_EQUAL(3, Si::get(s));
    BOOST_REQUIRE_EQUAL(Si::none, Si::get(s));
}

BOOST_AUTO_TEST_CASE(owning_source_type_erasure)
{
    std::vector<int> v = {1, 2, 3};
    auto s = Si::Source<int>::erase(Si::make_owning_source(v));
    BOOST_REQUIRE_EQUAL(1, Si::get(s));
    BOOST_REQUIRE_EQUAL(2, Si::get(s));
    BOOST_REQUIRE_EQUAL(3, Si::get(s));
    BOOST_REQUIRE_EQUAL(Si::none, Si::get(s));
}
