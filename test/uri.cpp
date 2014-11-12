#include <silicium/http/uri.hpp>
#include <silicium/memory_range.hpp>
#include <boost/test/unit_test.hpp>

namespace
{
	std::string to_string(Si::http::uri::string str)
	{
		return std::string(str.begin(), str.end());
	}

	template <class T, class F>
	auto map(std::vector<T> const &in, F const &transform)
	{
		std::vector<decltype(transform(in.front()))> result;
		result.reserve(in.size());
		std::transform(in.begin(), in.end(), std::back_inserter(result), transform);
		return result;
	}
}

BOOST_AUTO_TEST_CASE(uri_parse_http)
{
	boost::optional<Si::http::uri> const parsed = Si::http::parse_uri(Si::make_c_str_range("http://localhost:8080/a/b?k=0#h"));
	BOOST_REQUIRE(parsed);
	BOOST_CHECK_EQUAL("http", to_string(parsed->scheme));
	BOOST_CHECK_EQUAL("k=0", to_string(parsed->query));
	BOOST_CHECK_EQUAL("h", to_string(parsed->fragment));
	std::vector<std::string> const expected_path
	{
		"a", "b"
	};
	std::vector<std::string> const parsed_path = map(parsed->path, to_string);
	BOOST_CHECK_EQUAL_COLLECTIONS(expected_path.begin(), expected_path.end(), parsed_path.begin(), parsed_path.end());
}
