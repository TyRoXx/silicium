#include <silicium/pipe.hpp>
#include <silicium/read.hpp>
#include <silicium/write.hpp>
#include <boost/test/unit_test.hpp>
#include <array>

BOOST_AUTO_TEST_CASE(write_invalid)
{
    Si::error_or<std::size_t> result =
        Si::write(Si::no_file_handle, Si::make_c_str_range("test"));
    BOOST_CHECK(result.is_error());
}

BOOST_AUTO_TEST_CASE(write_empty)
{
    auto buffer = Si::make_pipe().move_value();
    Si::error_or<std::size_t> result =
        Si::write(buffer.write.handle, Si::make_c_str_range(""));
    BOOST_REQUIRE(!result.is_error());
    BOOST_CHECK_EQUAL(0U, result.get());
}

BOOST_AUTO_TEST_CASE(write_non_empty)
{
    auto buffer = Si::make_pipe().move_value();
    auto const message = Si::make_c_str_range("test");
    Si::error_or<std::size_t> result = Si::write(buffer.write.handle, message);
    BOOST_REQUIRE(!result.is_error());
    BOOST_CHECK_EQUAL(static_cast<size_t>(message.size()), result.get());
    std::array<char, 4096> read_buffer;
    result =
        Si::read(buffer.read.handle, Si::make_contiguous_range(read_buffer));
    BOOST_REQUIRE(!result.is_error());
    BOOST_REQUIRE_EQUAL(static_cast<size_t>(message.size()), result.get());
    BOOST_CHECK_EQUAL_COLLECTIONS(message.begin(), message.end(),
                                  read_buffer.begin(),
                                  read_buffer.begin() + result.get());
}
