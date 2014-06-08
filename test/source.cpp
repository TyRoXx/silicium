#include <silicium/source.hpp>
#include <boost/test/unit_test.hpp>

namespace Si
{
	BOOST_AUTO_TEST_CASE(line_source_empty)
	{
		memory_source<char> empty;
		line_source lines(empty);
		BOOST_CHECK_EQUAL(0, lines.minimum_size());
		BOOST_REQUIRE(lines.maximum_size());
		BOOST_CHECK_EQUAL(static_cast<boost::uintmax_t>(0), *lines.maximum_size());
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
		BOOST_REQUIRE(lines.maximum_size());
		BOOST_CHECK_EQUAL(static_cast<boost::uintmax_t>(original.size()), *lines.maximum_size());
		BOOST_CHECK(lines.map_next(1).empty());
		std::vector<char> line;
		auto * const result = lines.copy_next(boost::make_iterator_range(&line, &line + 1));
		BOOST_CHECK_EQUAL(&line + 1, result);
		BOOST_CHECK_EQUAL("abc", std::string(begin(line), end(line)));
	}

	BOOST_AUTO_TEST_CASE(buffering_source_empty)
	{
		memory_source<char> source;
		buffering_source<char> buffer(source, 1);
		BOOST_CHECK(buffer.map_next(1).empty());
		char c = 0;
		BOOST_CHECK_EQUAL(&c, buffer.copy_next(boost::make_iterator_range(&c, &c + 1)));
		BOOST_CHECK_EQUAL(0, buffer.skip(1));
	}

	BOOST_AUTO_TEST_CASE(buffering_source_non_empty)
	{
		std::string const original = "x";
		memory_source<char> source(boost::make_iterator_range(original.data(), original.data() + original.size()));
		buffering_source<char> buffer(source, 1);
		auto mapped = buffer.map_next_mutable(1);
		BOOST_REQUIRE_EQUAL(1, mapped.size());
		BOOST_CHECK_EQUAL('x', mapped.front());
		char c = 0;
		BOOST_CHECK_EQUAL(&c + 1, buffer.copy_next(boost::make_iterator_range(&c, &c + 1)));
		BOOST_CHECK_EQUAL(0, buffer.skip(1));
	}

	BOOST_AUTO_TEST_CASE(mutable_source_iterator_empty)
	{
		memory_source<char> source;
		buffering_source<char> buffer(source, 1);
		mutable_source_iterator<char> begin(buffer), end;
		BOOST_CHECK(begin == end);
	}

	BOOST_AUTO_TEST_CASE(mutable_source_iterator_non_empty)
	{
		std::string const original = "x";
		memory_source<char> source(boost::make_iterator_range(original.data(), original.data() + original.size()));
		buffering_source<char> buffer(source, 1);
		mutable_source_iterator<char> begin(buffer), end;
		BOOST_REQUIRE(begin != end);
		BOOST_CHECK_EQUAL('x', *begin);
		++begin;
		BOOST_CHECK(begin == end);
	}

	struct element
	{
	};

	BOOST_AUTO_TEST_CASE(generator_source_empty)
	{
		auto source = Si::make_generator_source<element>([]() -> boost::optional<element>
		{
			return boost::none;
		});
		BOOST_CHECK(!Si::get(source));
	}

	BOOST_AUTO_TEST_CASE(generator_source_limited)
	{
		bool first = true;
		auto source = Si::make_generator_source<int>([&first]() -> boost::optional<int>
		{
			auto result = first ? boost::make_optional(12) : boost::none;
			first = false;
			return result;
		});
		BOOST_REQUIRE(12 == Si::get(source));
		BOOST_REQUIRE(!Si::get(source));
		BOOST_REQUIRE(!Si::get(source));
	}

	BOOST_AUTO_TEST_CASE(generator_source_never_empty)
	{
		int next = 0;
		auto source = Si::make_generator_source<int>([&next]() -> boost::optional<int>
		{
			return next++;
		});
		auto buffer = Si::make_buffer(source, 2);
		for (int i = 0; i < 10; ++i)
		{
			BOOST_REQUIRE(i == Si::get(buffer));
		}
	}
}
