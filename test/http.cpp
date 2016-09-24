#include <silicium/http/http.hpp>
#include <silicium/http/request_parser_sink.hpp>
#include <silicium/source/memory_source.hpp>
#include <silicium/variant.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <silicium/sink/ptr_sink.hpp>
#include <silicium/error_or.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <boost/assign/list_of.hpp>

BOOST_AUTO_TEST_CASE(http_parse_header)
{
    std::string const incoming = "GET / HTTP/1.0\r\n"
                                 "Key: Value\r\n"
                                 "\r\n";
    auto source = Si::make_container_source(incoming);
    Si::optional<Si::http::request> const parsed =
        Si::http::parse_request(source);
    BOOST_REQUIRE(parsed);
    BOOST_CHECK_EQUAL("GET", parsed->method);
    BOOST_CHECK_EQUAL("/", parsed->path);
    BOOST_CHECK_EQUAL("HTTP/1.0", parsed->http_version);
    std::map<Si::noexcept_string, Si::noexcept_string> const
        expected_arguments =
            boost::assign::list_of(std::make_pair("Key", "Value"));
    BOOST_CHECK(expected_arguments == parsed->arguments);
}

BOOST_AUTO_TEST_CASE(http_write_request_header)
{
    std::string generated;
    auto sink = Si::make_container_sink(generated);
    Si::http::request header;
    header.http_version = "HTTP/1.1";
    header.method = "POST";
    header.path = "/p";
    header.arguments["Content-Length"] = "13";
    generate_request(sink, header);
    BOOST_CHECK_EQUAL("POST /p HTTP/1.1\r\n"
                      "Content-Length: 13\r\n"
                      "\r\n",
                      generated);
}

BOOST_AUTO_TEST_CASE(response_header_move)
{
    Si::http::response a;
    Si::http::response b(std::move(a));
    a = std::move(b);
}

BOOST_AUTO_TEST_CASE(response_header_copy)
{
    Si::http::response a;
    Si::http::response b(a);
    a = b;
}

#if SILICIUM_HAS_VARIANT
BOOST_AUTO_TEST_CASE(response_header_compatible_with_variant)
{
    Si::variant<Si::http::response> v;
    auto w = v;
    w = std::move(v);
    auto u = std::move(w);
    u = v;
}
#endif

BOOST_AUTO_TEST_CASE(http_request_parser_sink)
{
    std::vector<Si::http::request> results;
    auto parser =
        Si::http::make_request_parser_sink(Si::make_container_sink(results));
    Si::append(parser, "GET / HTTP/1.1\r\n");
    BOOST_CHECK(results.empty());
    Si::append(parser, "Host: host\r\n");
    BOOST_CHECK(results.empty());
    Si::append(parser, "\r\n");
    BOOST_REQUIRE_EQUAL(1u, results.size());
    Si::http::request &result = results[0];
    BOOST_CHECK_EQUAL("GET", result.method);
    BOOST_CHECK_EQUAL("/", result.path);
    BOOST_CHECK_EQUAL("HTTP/1.1", result.http_version);
    BOOST_CHECK_EQUAL(1u, result.arguments.size());
    {
        std::map<Si::noexcept_string, Si::noexcept_string> expected_arguments;
        expected_arguments.insert(std::make_pair("Host", "host"));
        BOOST_CHECK(expected_arguments == result.arguments);
    }
}