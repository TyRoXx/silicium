#include <silicium/write_file.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(write_invalid)
{
	Si::error_or<std::size_t> result = Si::write(Si::no_file_handle, Si::make_c_str_range("test"));
	BOOST_CHECK(result.is_error());
}
