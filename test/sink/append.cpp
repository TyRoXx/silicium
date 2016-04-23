#include <silicium/sink/append.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(append_long)
{
    std::vector<long> output;
    auto sink = Si::make_container_sink(output);
    Si::append(sink, 1L);
    Si::append(sink, 2);
    Si::append(sink, static_cast<short>(3));
    std::array<long, 3> const expected = {{1, 2, 3}};
    BOOST_CHECK_EQUAL_COLLECTIONS(
        expected.begin(), expected.end(), output.begin(), output.end());
}

BOOST_AUTO_TEST_CASE(append_string)
{
    std::vector<std::string> output;
    auto sink = Si::make_container_sink(output);
    Si::append(sink, std::string("a"));
    Si::append(sink, "b");
    Si::append(sink, static_cast<char const *>("c"));
    std::array<std::string, 3> const expected = {{"a", "b", "c"}};
    BOOST_CHECK_EQUAL_COLLECTIONS(
        expected.begin(), expected.end(), output.begin(), output.end());
}

BOOST_AUTO_TEST_CASE(append_char_to_vector)
{
    std::vector<char> output;
    auto sink = Si::make_container_sink(output);
    Si::append(sink, 'a');
    Si::append(sink, Si::make_c_str_range("b"));
    Si::append(sink, static_cast<char const *>("c"));
    Si::append(sink, std::string("d"));
    Si::append(sink, boost::container::string("e"));
    Si::append(sink,
#if BOOST_VERSION >= 105300
               boost::string_ref
#else
               Si::make_c_str_range
#endif
               ("f"));
    std::string const expected = "abcdef";
    BOOST_CHECK_EQUAL_COLLECTIONS(
        expected.begin(), expected.end(), output.begin(), output.end());
}

BOOST_AUTO_TEST_CASE(append_char_to_std_string)
{
    std::string output;
    auto sink = Si::make_container_sink(output);
    Si::append(sink, 'a');
    Si::append(sink, Si::make_c_str_range("b"));
    Si::append(sink, static_cast<char const *>("c"));
    Si::append(sink, std::string("d"));
    Si::append(sink, boost::container::string("e"));
    Si::append(sink,
#if BOOST_VERSION >= 105300
               boost::string_ref
#else
               Si::make_c_str_range
#endif
               ("f"));
    std::string const expected = "abcdef";
    BOOST_CHECK_EQUAL_COLLECTIONS(
        expected.begin(), expected.end(), output.begin(), output.end());
}

BOOST_AUTO_TEST_CASE(append_char_to_boost_string)
{
    boost::container::string output;
    auto sink = Si::make_container_sink(output);
    Si::append(sink, 'a');
    Si::append(sink, Si::make_c_str_range("b"));
    Si::append(sink, static_cast<char const *>("c"));
    Si::append(sink, std::string("d"));
    Si::append(sink, boost::container::string("e"));
    Si::append(sink,
#if BOOST_VERSION >= 105300
               boost::string_ref
#else
               Si::make_c_str_range
#endif
               ("f"));
    std::string const expected = "abcdef";
    BOOST_CHECK_EQUAL_COLLECTIONS(
        expected.begin(), expected.end(), output.begin(), output.end());
}
