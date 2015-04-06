#include <silicium/html.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(html_string_string)
{
	std::string html;
	Si::html::write_string(Si::make_container_sink(html), "123<45>&'\"");
	BOOST_CHECK_EQUAL("123&lt;45&gt;&amp;&apos;&quot;", html);
}

BOOST_AUTO_TEST_CASE(html_generator_paired_tags)
{
	std::string html;
	auto gen = Si::html::make_generator(Si::make_container_sink(html));
	gen.write("1<2");
	gen.element("b", [&gen]()
	{
		gen.write("abc & \"");
	});
	BOOST_CHECK_EQUAL("1&lt;2<b>abc &amp; &quot;</b>", html);
}

BOOST_AUTO_TEST_CASE(html_generator_tags_with_attributes)
{
	std::string html;
	auto gen = Si::html::make_generator(Si::make_container_sink(html));
	gen.element(
		"b",
		[&gen]()
		{
			gen.attribute("a", "1");
			gen.attribute("b", "\"");
		},
		[&gen]()
		{
			gen.write("abc");
		}
	);
	BOOST_CHECK_EQUAL("<b a=\"1\" b=\"&quot;\">abc</b>", html);
}
