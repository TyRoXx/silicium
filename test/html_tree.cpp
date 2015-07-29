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
				tag("input", detail::make_element<min_length<0>>([](Si::sink<char, Si::success> &destination)
					{
						Si::html::add_attribute(destination, "key", "value");
					}),
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
		tag("input", detail::make_element<min_length<0>>([](Si::sink<char, Si::success> &destination)
			{
				Si::html::add_attribute(destination, "key", "value");
			}),
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
		tag("input", detail::make_element<min_length<0>>([](Si::sink<char, Si::success> &destination)
			{
				Si::html::add_attribute(destination, "key", "value");
			}),
			text("content")
		);
	BOOST_CHECK_EQUAL(22u, decltype(document)::length_type::value);
	std::string generated = generate<std::string>(document);
	BOOST_CHECK_EQUAL("<input key=\"value\">content</input>", generated);
}

BOOST_AUTO_TEST_CASE(html_tree_one_attribute)
{
	using namespace Si::html;
	auto document =
		tag("i",
			attribute("key", "value"),
			empty
		);
	BOOST_CHECK_EQUAL(16u, decltype(document)::length_type::value);
	std::string generated = generate<std::string>(document);
	BOOST_CHECK_EQUAL("<i key=\"value\"/>", generated);
}

BOOST_AUTO_TEST_CASE(html_tree_two_attributes)
{
	using namespace Si::html;
	auto document =
		tag("i",
			attribute("key", "value") + attribute("key2", "value2"),
			empty
		);
	BOOST_CHECK_EQUAL(30u, decltype(document)::length_type::value);
	std::string generated = generate<std::string>(document);
	BOOST_CHECK_EQUAL("<i key=\"value\" key2=\"value2\"/>", generated);
}

BOOST_AUTO_TEST_CASE(html_tree_trait)
{
	Si::html::Element<>::box const erased = Si::html::Element<>::make_box(Si::html::tag("test", Si::html::text("Hello")));
	std::string generated;
	auto sink = Si::Sink<char, Si::success>::erase(Si::make_container_sink(generated));
	erased.generate(sink);
	BOOST_CHECK_EQUAL("<test>Hello</test>", generated);
}


BOOST_AUTO_TEST_CASE(html_tree_produces_same_output_as_generator)
{
	bool const build_triggered = true;

	std::vector<char> old_style_generated;
	auto html = Si::html::make_generator(Si::make_container_sink(old_style_generated));
	html("html", [&]
	{
		html("head", [&]
		{
			html("title", [&]
			{
				html.write("Silicium build tester");
			});
		});
		html("body", [&]
		{
			if (build_triggered)
			{
				html.write("build was triggered");
			}
			html("form",
				[&]
			{
				html.attribute("action", "/");
				html.attribute("method", "POST");
			},
				[&]
			{
				html("input",
					[&]
				{
					html.attribute("type", "submit");
					html.attribute("value", "Trigger build");
				},
					Si::html::empty);
			});
		});

	});

	using namespace Si::html;
	auto document = tag("html",
		tag("head",
			tag("title",
				text("Silicium build tester")
			)
		) +
		tag("body",
			dynamic<min_length<0>>([build_triggered](Si::sink<char, Si::success> &destination)
			{
				if (!build_triggered)
				{
					return;
				}
				text("build was triggered").generate(destination);
			}) +
			tag("form",
				attribute("action", "/") +
				attribute("method", "POST"),
				tag("input",
					attribute("type", "submit") +
					attribute("value", "Trigger build"),
					empty
				)
			)
		)
	);
	std::vector<char> new_style_generated = Si::html::generate<std::vector<char>>(document);

	BOOST_CHECK(old_style_generated == new_style_generated);
}
