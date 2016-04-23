#include <boost/test/unit_test.hpp>
#include <silicium/memory_range.hpp>
#include <boost/array.hpp>
#include <boost/assign/list_of.hpp>
#include <array>

BOOST_AUTO_TEST_CASE(make_memory_range_c_str_literal)
{
    Si::iterator_range<char const *> r = Si::make_memory_range("hello");
    std::array<char, 6> const expected = {{'h', 'e', 'l', 'l', 'o',
                                           // the terminating zero is expected
                                           // because std::end("hello") points
                                           // after \0, not at it
                                           '\0'}};
    BOOST_CHECK_EQUAL_COLLECTIONS(
        expected.begin(), expected.end(), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_c_array)
{
    using std::begin;
    using std::end;
    char expected[3] = {1, 2, 3};
    Si::iterator_range<char *> r = Si::make_memory_range(expected);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        begin(expected), end(expected), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_const_c_array)
{
    using std::begin;
    using std::end;
    char const expected[3] = {1, 2, 3};
    Si::iterator_range<char const *> r = Si::make_memory_range(expected);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        begin(expected), end(expected), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_std_array)
{
    std::array<char, 3> expected = {{1, 2, 3}};
    Si::iterator_range<char *> r = Si::make_memory_range(expected);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        begin(expected), end(expected), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_const_std_array)
{
    std::array<char, 3> const expected = {{1, 2, 3}};
    Si::iterator_range<char const *> r = Si::make_memory_range(expected);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        begin(expected), end(expected), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_boost_array)
{
    using std::begin;
    using std::end;
    boost::array<char, 3> expected = {{1, 2, 3}};
    Si::iterator_range<char *> r = Si::make_memory_range(expected);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        begin(expected), end(expected), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_const_boost_array)
{
    using std::begin;
    using std::end;
    boost::array<char, 3> const expected = {{1, 2, 3}};
    Si::iterator_range<char const *> r = Si::make_memory_range(expected);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        begin(expected), end(expected), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_std_vector)
{
    std::vector<char> expected = boost::assign::list_of(1)(2)(3);
    Si::iterator_range<char *> r = Si::make_memory_range(expected);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        begin(expected), end(expected), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_const_std_vector)
{
    std::vector<char> const expected = boost::assign::list_of(1)(2)(3);
    Si::iterator_range<char const *> r = Si::make_memory_range(expected);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        begin(expected), end(expected), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_std_string)
{
    std::string expected = "123";
    Si::iterator_range<char *> r = Si::make_memory_range(expected);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        begin(expected), end(expected), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_const_std_string)
{
    std::string const expected = "123";
    Si::iterator_range<char const *> r = Si::make_memory_range(expected);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        begin(expected), end(expected), r.begin(), r.end());
}

#if BOOST_VERSION >= 105300
#include <boost/utility/string_ref.hpp>

BOOST_AUTO_TEST_CASE(make_memory_range_string_ref)
{
    boost::string_ref expected = "123";
    Si::iterator_range<char const *> r = Si::make_memory_range(expected);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        begin(expected), end(expected), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_const_string_ref)
{
    boost::string_ref const expected = "123";
    Si::iterator_range<char const *> r = Si::make_memory_range(expected);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        begin(expected), end(expected), r.begin(), r.end());
}
#endif

BOOST_AUTO_TEST_CASE(make_memory_range_from_pointers)
{
    std::vector<char> expected = boost::assign::list_of(1)(2)(3);
    Si::iterator_range<char *> r = Si::make_memory_range(
        expected.data(), expected.data() + expected.size());
    BOOST_CHECK_EQUAL_COLLECTIONS(
        begin(expected), end(expected), r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(make_memory_range_from_pointers_to_const)
{
    std::vector<char> const expected = boost::assign::list_of(1)(2)(3);
    Si::iterator_range<char const *> r = Si::make_memory_range(
        expected.data(), expected.data() + expected.size());
    BOOST_CHECK_EQUAL_COLLECTIONS(
        begin(expected), end(expected), r.begin(), r.end());
}
