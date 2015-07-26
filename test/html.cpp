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

BOOST_AUTO_TEST_CASE(html_raw)
{
	std::string html;
	auto gen = Si::html::make_generator(Si::make_container_sink(html));
	auto const content = "<tag attribute=\"1\">";
	gen.raw(content);
	BOOST_CHECK_EQUAL(content, html);
}

BOOST_AUTO_TEST_CASE(html_empty)
{
	std::string html;
	auto gen = Si::html::make_generator(Si::make_container_sink(html));
	gen.element("tag", Si::html::empty);
	BOOST_CHECK_EQUAL("<tag/>", html);
}

BOOST_AUTO_TEST_CASE(html_empty_with_attribute)
{
	std::string html;
	auto gen = Si::html::make_generator(Si::make_container_sink(html));
	gen.element("tag", [&]
	{
		gen.attribute("attribute", "2");
	},
		Si::html::empty);
	BOOST_CHECK_EQUAL("<tag attribute=\"2\"/>", html);
}

namespace Si
{
	namespace html2
	{
		namespace detail
		{
			template <class ContentGenerator>
			struct element
			{
				ContentGenerator generate;
				std::size_t min_length;
			};

			template <class ContentGenerator>
			auto make_element(ContentGenerator &&generate, std::size_t min_length)
			{
				return element<typename std::decay<ContentGenerator>::type>{std::forward<ContentGenerator>(generate), min_length};
			}
		}

		template <std::size_t NameLength, class Element>
		auto tag(char const (&name)[NameLength], Element &&content)
		{
			std::size_t const content_min_length = content.min_length;
			return detail::make_element([
				&name,
				SILICIUM_CAPTURE_EXPRESSION(content, std::forward<Element>(content))
			] (sink<char, success> &destination)
			{
				html::open_element(destination, name);
				content.generate(destination);
				html::close_element(destination, name);
			}, 1 + (NameLength - 1) + 1 + content_min_length + 2 + (NameLength - 1) + 1);
		}

		auto text(char const *content)
		{
			return detail::make_element([content](sink<char, success> &destination)
			{
				html::write_string(destination, content);
			}, 0);
		}

		template <std::size_t Length>
		auto raw(char const (&content)[Length])
		{
			return detail::make_element([&content](sink<char, success> &destination)
			{
				Si::append(destination, content);
			}, (Length - 1));
		}

		auto sequence()
		{
			return detail::make_element([](sink<char, success> &)
			{
			}, 0);
		}

		template <class Head, class ...Tail>
		auto sequence(Head &&head, Tail &&...tail)
		{
			using html2::sequence;
			auto tail_elements = sequence(std::forward<Tail>(tail)...);
			std::size_t const min_length = head.min_length + tail_elements.min_length;
			return detail::make_element([
				SILICIUM_CAPTURE_EXPRESSION(head, std::forward<Head>(head)),
				SILICIUM_CAPTURE_EXPRESSION(tail_elements, std::move(tail_elements))
				] (sink<char, success> &destination)
			{
				head.generate(destination);
				tail_elements.generate(destination);
			}, min_length);
		}

		namespace detail
		{
			template <class ContentGenerator1, class ContentGenerator2>
			auto operator + (element<ContentGenerator1> &&left, element<ContentGenerator2> &&right)
			{
				return sequence(std::move(left), std::move(right));
			}
		}
	}
}

BOOST_AUTO_TEST_CASE(html2_shorter_syntax_experiments)
{
	using namespace Si::html2;
	auto document =
		tag("html",
			tag("head",
				tag("title",
					text("Title")
				)
			)
			+
			tag("body",
				text("Hello, ") + raw("<b>world</b>")
			)
		);
	std::string generated;
	auto sink = Si::Sink<char, Si::success>::erase(Si::make_container_sink(generated));
	BOOST_CHECK_EQUAL(std::strlen("<html><head><title>Title</title></head><body>Hello, </body></html>"), document.min_length);
	document.generate(sink);
	BOOST_CHECK_EQUAL("<html><head><title>Title</title></head><body>Hello, <b>world</b></body></html>", generated);
}
