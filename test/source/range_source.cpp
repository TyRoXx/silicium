#include <silicium/source/range_source.hpp>
#include <silicium/memory_range.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/range/istream_range.hpp>

BOOST_AUTO_TEST_CASE(range_source_random_access)
{
    auto source = Si::make_range_source(Si::make_c_str_range("Test"));
    BOOST_CHECK_EQUAL('T', Si::get(source));
    BOOST_CHECK_EQUAL('e', Si::get(source));
    BOOST_CHECK_EQUAL('s', Si::get(source));
    BOOST_CHECK_EQUAL('t', Si::get(source));
    BOOST_CHECK_EQUAL(Si::none, Si::get(source));
}

BOOST_AUTO_TEST_CASE(range_source_single_pass)
{
    std::istringstream buffer("Test");
    auto source =
        Si::make_range_source(boost::range::istream_range<char>(buffer));
    BOOST_CHECK_EQUAL('T', Si::get(source));
    BOOST_CHECK_EQUAL('e', Si::get(source));
    BOOST_CHECK_EQUAL('s', Si::get(source));
    BOOST_CHECK_EQUAL('t', Si::get(source));
    BOOST_CHECK_EQUAL(Si::none, Si::get(source));
}
