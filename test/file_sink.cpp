#include <silicium/file_sink.hpp>
#include <silicium/open.hpp>
#include <silicium/read_file.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(file_sink_success)
{
	boost::filesystem::path const file_name = "/tmp/silicium_file_sink_success.txt";
	{
		Si::file_descriptor file = Si::overwrite_file(file_name).get();
		Si::file_sink sink(file.handle);
		BOOST_CHECK_EQUAL(boost::system::error_code(), Si::append(sink, Si::file_sink_element{Si::make_c_str_range("test")}));
		BOOST_CHECK_EQUAL(boost::system::error_code(), Si::append(sink, Si::file_sink_element{Si::flush{}}));
		BOOST_CHECK_EQUAL(boost::system::error_code(), Si::append(sink, Si::file_sink_element{Si::seek_request{Si::seek_set{4}}}));
		BOOST_CHECK_EQUAL(boost::system::error_code(), Si::append(sink, Si::file_sink_element{Si::seek_request{Si::seek_add{-1}}}));
		std::array<Si::file_sink_element, 3> write_vector
		{{
			Si::make_c_str_range("aaa"),
			Si::make_c_str_range("bbbb"),
			Si::make_c_str_range("ccccc")
		}};
		BOOST_CHECK_EQUAL(boost::system::error_code(), sink.append(boost::make_iterator_range(write_vector.data(), write_vector.data() + write_vector.size())));
	}
	std::vector<char> content = Si::read_file(file_name);
	std::string const expected = "tesaaabbbbccccc";
	BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), content.begin(), content.end());
}

BOOST_AUTO_TEST_CASE(file_sink_error)
{
	auto file = Si::open_reading("/proc/cpuinfo").get();
	Si::file_sink sink(file.handle);
	BOOST_CHECK_EQUAL(boost::system::error_code(9, boost::system::system_category()), Si::append(sink, Si::file_sink_element{Si::make_c_str_range("test")}));
	BOOST_CHECK_EQUAL(boost::system::error_code(22, boost::system::system_category()), Si::append(sink, Si::file_sink_element{Si::flush{}}));
	BOOST_CHECK_EQUAL(boost::system::error_code(22, boost::system::system_category()), Si::append(sink, Si::file_sink_element{Si::seek_request{Si::seek_add{-4}}}));
	BOOST_CHECK_EQUAL(boost::system::error_code(), Si::append(sink, Si::file_sink_element{Si::seek_request{Si::seek_set{2}}}));
}
