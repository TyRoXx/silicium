#ifndef SILICIUM_SOURCE_HPP
#define SILICIUM_SOURCE_HPP

#include <silicium/override.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/optional.hpp>
#include <boost/concept_check.hpp>
#include <boost/cstdint.hpp>

namespace Si
{
	template <class Element>
	struct source
	{
		using element_type = Element;

		virtual ~source()
		{
		}

		virtual boost::iterator_range<element_type const *> map_next(std::size_t size) = 0;
		virtual element_type *copy_next(boost::iterator_range<element_type *> destination) = 0;
	};

	template <class X>
	struct Source
	{
		using element_type = typename X::element_type;

		BOOST_CONCEPT_USAGE(Source)
		{
			X move_constructed = std::move(source);
			boost::ignore_unused_variable_warning(move_constructed);
			source = std::move(move_constructed);
			X default_constructible;
			boost::ignore_unused_variable_warning(default_constructible);
			boost::iterator_range<element_type const *> mapped = source.map_next(std::size_t(0));
			boost::ignore_unused_variable_warning(mapped);
			element_type * const copied = source.copy_next(boost::iterator_range<element_type *>());
			boost::ignore_unused_variable_warning(copied);
		}

	private:

		X source;
		element_type element;
	};

	namespace detail
	{
		//example and test for the smallest possible Source
		struct minimum_source
		{
			using element_type = int;
			boost::iterator_range<element_type const *> map_next(std::size_t size);
			element_type *copy_next(boost::iterator_range<element_type *> destination);
			boost::uintmax_t minimum_size();
			boost::optional<boost::uintmax_t> maximum_size();
			std::size_t skip(std::size_t count);
		};

		BOOST_CONCEPT_ASSERT((Source<minimum_source>));
	}

	template <class Source>
	boost::optional<typename Source::element_type> get(Source &from)
	{
		typename Source::element_type result;
		if (&result == from.copy_next(boost::make_iterator_range(&result, &result + 1)))
		{
			return boost::none;
		}
		return std::move(result);
	}

	template <class Container>
	auto data(Container &container) -> decltype(&container[0])
	{
		return container.empty() ? nullptr : &container[0];
	}

	template <class Element, class Sequence = std::vector<Element>>
	auto take(source<Element> &from, std::size_t count) -> Sequence
	{
		Sequence taken;
		taken.resize(count);
		auto end = from.copy_next(boost::make_iterator_range(data(taken), data(taken) + taken.size()));
		taken.resize(std::distance(data(taken), end));
		return taken;
	}
}

#endif
