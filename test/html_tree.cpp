#include <silicium/html/tree.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(html_tree)
{
	using namespace Si::html;
	auto document =
		tag("html",
			tag("head",
				tag("title",
					text("Title")
				)
			)
			+
			tag("body",
				text("Hello, ") + raw("<b>world</b>") + dynamic<min_length<0>>([](Si::sink<char, Si::success> &destination)
				{
					Si::html::unpaired_element(destination, "br");
				})+
				tag("input", [](Si::sink<char, Si::success> &destination)
					{
						Si::html::add_attribute(destination, "key", "value");
					},
					min_length<0>{},
					empty
				)
			)
		);
	BOOST_CHECK_EQUAL(86u, decltype(document)::length_type::value);
	std::string generated = generate<std::string>(document);
	BOOST_CHECK_EQUAL("<html><head><title>Title</title></head><body>Hello, <b>world</b><br/><input key=\"value\"/></body></html>", generated);
}

BOOST_AUTO_TEST_CASE(html_tree_attributes_of_unpaired_tag)
{
	using namespace Si::html;
	auto document =
		tag("input", [](Si::sink<char, Si::success> &destination)
			{
				Si::html::add_attribute(destination, "key", "value");
			},
			min_length<0>{},
			empty
		);
	BOOST_CHECK_EQUAL(8u, decltype(document)::length_type::value);
	std::string generated = generate<std::string>(document);
	BOOST_CHECK_EQUAL("<input key=\"value\"/>", generated);
}

BOOST_AUTO_TEST_CASE(html_tree_attributes)
{
	using namespace Si::html;
	auto document =
		tag("input", [](Si::sink<char, Si::success> &destination)
			{
				Si::html::add_attribute(destination, "key", "value");
			},
			min_length<0>{},
			text("content")
		);
	BOOST_CHECK_EQUAL(22u, decltype(document)::length_type::value);
	std::string generated = generate<std::string>(document);
	BOOST_CHECK_EQUAL("<input key=\"value\">content</input>", generated);
}

