#include <silicium/read.hpp>
#include <silicium/write.hpp>
#include <silicium/pipe.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(read_invalid)
{
    std::array<char, 1024> buffer;
    Si::error_or<std::size_t> result =
        Si::read(Si::no_file_handle, Si::make_memory_range(buffer));
    BOOST_CHECK(result.is_error());
}

BOOST_AUTO_TEST_CASE(read_write_pipe)
{
    std::array<char, 1024> buffer;
    auto pipe = Si::make_pipe().move_value();
    auto const expected = Si::make_c_str_range("Hello");
    BOOST_CHECK_EQUAL(
        Si::error_or<std::size_t>(static_cast<std::size_t>(expected.size())),
        Si::write(pipe.write.handle, expected));
    Si::error_or<std::size_t> result =
        Si::read(pipe.read.handle, Si::make_memory_range(buffer));
    BOOST_REQUIRE(!result.is_error());
    BOOST_REQUIRE_EQUAL(
        static_cast<std::size_t>(expected.size()), result.get());
    BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(),
                                  buffer.begin(),
                                  buffer.begin() + result.get());
}
