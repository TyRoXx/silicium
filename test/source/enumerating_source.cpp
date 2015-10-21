#include <silicium/source/enumerating_source.hpp>
#include <silicium/source/memory_source.hpp>
#include <silicium/source/single_source.hpp>
#include <boost/test/unit_test.hpp>

namespace Si
{
	template <class Element>
	struct empty_source : Source<Element>::interface
	{
		typedef Element element_type;

		virtual iterator_range<element_type const *> map_next(std::size_t size) SILICIUM_OVERRIDE
		{
			boost::ignore_unused_variable_warning(size);
			return iterator_range<element_type const *>();
		}

		virtual element_type *copy_next(iterator_range<element_type *> destination) SILICIUM_OVERRIDE
		{
			boost::ignore_unused_variable_warning(destination);
			return destination.begin();
		}
	};
}

BOOST_AUTO_TEST_CASE(enumerating_source_directly_empty)
{
	auto s = Si::make_enumerating_source(Si::empty_source<Si::iterator_range<int *>>());
	Si::optional<int> e = Si::get(s);
	BOOST_CHECK(!e);
}

BOOST_AUTO_TEST_CASE(enumerating_source_indirectly_empty)
{
	auto s = Si::make_enumerating_source(Si::make_single_source(Si::iterator_range<int *>()));
	Si::optional<int> e = Si::get(s);
	BOOST_CHECK(!e);
}

BOOST_AUTO_TEST_CASE(enumerating_source_non_empty)
{
	int const element = 3;
	auto s =
	    Si::make_enumerating_source(Si::make_single_source(Si::iterator_range<int const *>(&element, &element + 1)));
	Si::optional<int> e = Si::get(s);
	BOOST_REQUIRE(e);
	BOOST_CHECK_EQUAL(3, e);
}

BOOST_AUTO_TEST_CASE(enumerating_source_map_next_at_the_end)
{
	int const element = 3;
	auto s =
	    Si::make_enumerating_source(Si::make_single_source(Si::iterator_range<int const *>(&element, &element + 1)));
	Si::iterator_range<int const *> m = s.map_next(2);
	BOOST_CHECK_EQUAL(&element, m.begin());
	BOOST_CHECK_EQUAL(1, m.size());
	BOOST_CHECK(!Si::get(s));
}
