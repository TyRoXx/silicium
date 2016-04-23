#include <silicium/get_last_error.hpp>
#include <silicium/write.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(get_last_error)
{
    Si::error_or<std::size_t> const result =
        Si::write(Si::no_file_handle, Si::make_c_str_range(""));
    boost::system::error_code const ec = Si::get_last_error();
    BOOST_CHECK_EQUAL(result.error(), ec);
    BOOST_CHECK(ec);
}
