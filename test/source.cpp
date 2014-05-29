#include <silicium/source.hpp>
#include <boost/test/unit_test.hpp>

namespace Si
{
	BOOST_AUTO_TEST_CASE(line_source_empty)
	{
		memory_source<char> empty;
		line_source lines(empty);
		BOOST_CHECK_EQUAL(0, lines.minimum_size());
		BOOST_CHECK_EQUAL(static_cast<boost::uintmax_t>(0), lines.maximum_size());
		BOOST_CHECK(lines.map_next(1).empty());
		std::vector<char> line;
		auto * const result = lines.copy_next(boost::make_iterator_range(&line, &line + 1));
		BOOST_CHECK_EQUAL(&line, result);
	}

	BOOST_AUTO_TEST_CASE(line_source_cr_lf)
	{
		std::string const original = "abc\r\n123";
		memory_source<char> source(boost::make_iterator_range(original.data(), original.data() + original.size()));
		line_source lines(source);
		BOOST_CHECK_EQUAL(0, lines.minimum_size());
		BOOST_CHECK_EQUAL(static_cast<boost::uintmax_t>(original.size()), lines.maximum_size());
		BOOST_CHECK(lines.map_next(1).empty());
		std::vector<char> line;
		auto * const result = lines.copy_next(boost::make_iterator_range(&line, &line + 1));
		BOOST_CHECK_EQUAL(&line + 1, result);
		BOOST_CHECK_EQUAL("abc", std::string(begin(line), end(line)));
	}
}
