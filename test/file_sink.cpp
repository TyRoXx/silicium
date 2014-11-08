#include <silicium/file_sink.hpp>
#include <silicium/open.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(file_sink_success)
{
	auto file = Si::open_read_write("/tmp/silicium_file_sink_success.txt").get();
	Si::file_sink sink(file.handle);
	BOOST_CHECK_EQUAL(boost::system::error_code(), Si::append(sink, Si::file_sink_element{Si::make_memory_range("test")}));
	BOOST_CHECK_EQUAL(boost::system::error_code(), Si::append(sink, Si::file_sink_element{Si::flush{}}));
	BOOST_CHECK_EQUAL(boost::system::error_code(), Si::append(sink, Si::file_sink_element{Si::seek_request{Si::seek_add{-4}}}));
	BOOST_CHECK_EQUAL(boost::system::error_code(), Si::append(sink, Si::file_sink_element{Si::seek_request{Si::seek_set{2}}}));
}

BOOST_AUTO_TEST_CASE(file_sink_error)
{
	auto file = Si::open_reading("/proc/cpuinfo").get();
	Si::file_sink sink(file.handle);
	BOOST_CHECK_EQUAL(boost::system::error_code(9, boost::system::system_category()), Si::append(sink, Si::file_sink_element{Si::make_memory_range("test")}));
	BOOST_CHECK_EQUAL(boost::system::error_code(22, boost::system::system_category()), Si::append(sink, Si::file_sink_element{Si::flush{}}));
	BOOST_CHECK_EQUAL(boost::system::error_code(22, boost::system::system_category()), Si::append(sink, Si::file_sink_element{Si::seek_request{Si::seek_add{-4}}}));
	BOOST_CHECK_EQUAL(boost::system::error_code(), Si::append(sink, Si::file_sink_element{Si::seek_request{Si::seek_set{2}}}));
}
