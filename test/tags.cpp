#include "html_generator/tags.hpp"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(checking_empty_html)
{
    std::string html_generated;
    auto erased_html_sink = Si::Sink<char, Si::success>::erase(
            Si::make_container_sink(html_generated));
    const auto &content = tags::menu(tags::p("Content")) + tags::footer(tags::p("Copyright"));
    tags::html(tags::head(tags::title("Title")) + tags::body(
            content)).generate(erased_html_sink);
    BOOST_CHECK_EQUAL("<html><head><title>Title</title></head><body><menu><p>Content</p></menu><footer><p>Copyright</p></footer></body></html>", html_generated);
}

BOOST_AUTO_TEST_CASE(tags_link)
{
	std::string html_generated;
	auto erased_html_sink = Si::Sink<char, Si::success>::erase(
	    Si::make_container_sink(html_generated));
	tags::link("https://", "google.com").generate(erased_html_sink);
	BOOST_CHECK_EQUAL(
	    "<a href=\"https://google.com\" target=\"_blank\">google.com</a>",
	    html_generated);
}

BOOST_AUTO_TEST_CASE(paragraph)
{
	std::string html_generated;
	auto erased_html_sink = Si::Sink<char, Si::success>::erase(
	    Si::make_container_sink(html_generated));
	tags::p(tags::cl("someclass"), Si::html::text("Testing p tag")).generate(erased_html_sink);
	BOOST_CHECK_EQUAL("<p class=\"someclass\">Testing p tag</p>", html_generated);
}

BOOST_AUTO_TEST_CASE(div_box)
{
    std::string html_generated;
    auto erased_html_sink = Si::Sink<char, Si::success>::erase(
            Si::make_container_sink(html_generated));
    tags::div(tags::cl(""), Si::html::text("Testing div tag")).generate(erased_html_sink);
    BOOST_CHECK_EQUAL("<div class=\"\">Testing div tag</div>", html_generated);
}

BOOST_AUTO_TEST_CASE(span)
{
    std::string html_generated;
    auto erased_html_sink = Si::Sink<char, Si::success>::erase(
            Si::make_container_sink(html_generated));
    tags::span(tags::cl(""), Si::html::text("Testing span tag")).generate(erased_html_sink);
    BOOST_CHECK_EQUAL("<span class=\"\">Testing span tag</span>", html_generated);
}

BOOST_AUTO_TEST_CASE(break_row)
{
	std::string html_generated;
	auto erased_html_sink = Si::Sink<char, Si::success>::erase(
	    Si::make_container_sink(html_generated));
	tags::br(tags::cl("clear")).generate(erased_html_sink);
	BOOST_CHECK_EQUAL("<br class=\"clear\"/>", html_generated);
}

BOOST_AUTO_TEST_CASE(h1)
{
    std::string html_generated;
    auto erased_html_sink = Si::Sink<char, Si::success>::erase(
            Si::make_container_sink(html_generated));
    tags::h1(Si::html::text("Test heading")).generate(erased_html_sink);
    BOOST_CHECK_EQUAL("<h1>Test heading</h1>", html_generated);
}

BOOST_AUTO_TEST_CASE(h2)
{
    std::string html_generated;
    auto erased_html_sink = Si::Sink<char, Si::success>::erase(
            Si::make_container_sink(html_generated));
    tags::h2(Si::html::text("Test heading")).generate(erased_html_sink);
    BOOST_CHECK_EQUAL("<h2>Test heading</h2>", html_generated);
}

BOOST_AUTO_TEST_CASE(h3)
{
    std::string html_generated;
    auto erased_html_sink = Si::Sink<char, Si::success>::erase(
            Si::make_container_sink(html_generated));
    tags::h3(Si::html::text("Test heading")).generate(erased_html_sink);
    BOOST_CHECK_EQUAL("<h3>Test heading</h3>", html_generated);
}

BOOST_AUTO_TEST_CASE(h4)
{
    std::string html_generated;
    auto erased_html_sink = Si::Sink<char, Si::success>::erase(
            Si::make_container_sink(html_generated));
    tags::h4(Si::html::text("Test heading")).generate(erased_html_sink);
    BOOST_CHECK_EQUAL("<h4>Test heading</h4>", html_generated);
}

BOOST_AUTO_TEST_CASE(pre_tag)
{
    std::string html_generated;
    auto erased_html_sink = Si::Sink<char, Si::success>::erase(
            Si::make_container_sink(html_generated));
    tags::pre(Si::html::text("Test heading")).generate(erased_html_sink);
    BOOST_CHECK_EQUAL("<pre>Test heading</pre>", html_generated);
}

BOOST_AUTO_TEST_CASE(ul)
{
    std::string html_generated;
    auto erased_html_sink = Si::Sink<char, Si::success>::erase(
            Si::make_container_sink(html_generated));
    tags::ul(tags::li(Si::html::text("Test heading"))).generate(erased_html_sink);
    BOOST_CHECK_EQUAL("<ul><li>Test heading</li></ul>", html_generated);
}