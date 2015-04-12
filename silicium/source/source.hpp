#ifndef SILICIUM_SOURCE_HPP
#define SILICIUM_SOURCE_HPP

#include <silicium/config.hpp>
#include <silicium/iterator_range.hpp>
#include <silicium/optional.hpp>
#include <boost/cstdint.hpp>
#include <vector>

namespace Si
{
	template <class Element>
	struct source
	{
		typedef Element element_type;

		virtual ~source()
		{
		}

		virtual iterator_range<element_type const *> map_next(std::size_t size) = 0;
		virtual element_type *copy_next(iterator_range<element_type *> destination) = 0;
	};

	template <class Source>
	Si::optional<typename Source::element_type> get(Source &from)
	{
		typename Source::element_type result;
		if (&result == from.copy_next(Si::make_iterator_range(&result, &result + 1)))
		{
			return Si::none;
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
		auto end = from.copy_next(make_iterator_range(data(taken), data(taken) + taken.size()));
		taken.resize(std::distance(data(taken), end));
		return taken;
	}
}

#endif
