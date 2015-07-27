#include <silicium/html/tree.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(html_generate)
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
				})
			)
		);
	BOOST_STATIC_ASSERT(std::is_same<decltype(document)::length_type, min_length<78>>::value);
	std::string generated = generate<std::string>(document);
	BOOST_CHECK_EQUAL("<html><head><title>Title</title></head><body>Hello, <b>world</b><br/></body></html>", generated);
}
