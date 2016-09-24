#include <silicium/memory_size.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(memory_size_char)
{
    static constexpr Si::memory_size<char, Si::bounded_int<std::size_t, 3, 3>>
        s = Si::make_memory_size<char>(Si::literal<std::size_t, 3>());
    std::array<char, s.value.value()> a;
    BOOST_TEST(3u == a.size());
}

BOOST_AUTO_TEST_CASE(memory_size_int)
{
    static constexpr Si::memory_size<char, Si::bounded_int<std::size_t, 12, 12>>
        s = Si::as_chars(Si::make_memory_size<boost::int32_t>(
            Si::literal<std::size_t, 3>()));
    std::array<char, s.value.value()> a;
    BOOST_TEST(12u == a.size());
}
