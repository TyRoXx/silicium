#ifndef SILICIUM_FILTER_SOURCE_HPP
#define SILICIUM_FILTER_SOURCE_HPP

#include <silicium/source.hpp>
#include <silicium/detail/proper_value_function.hpp>

namespace Si
{
	template <class Input, class Predicate>
	struct filter_source
	{
		using element_type = typename Input::element_type;

		filter_source()
		{
		}

		filter_source(Input input, Predicate is_propagated)
			: input(std::move(input))
			, is_propagated(std::move(is_propagated))
		{
		}

		boost::iterator_range<element_type const *> map_next(std::size_t size)
		{
			boost::ignore_unused_variable_warning(size);
			throw std::logic_error("todo");
		}

		element_type *copy_next(boost::iterator_range<element_type *> destination)
		{
			element_type *copied = destination.begin();
			for (; copied != destination.end(); )
			{
				auto element = Si::get(input);
				if (!element)
				{
					break;
				}
				if (is_propagated(*element))
				{
					*copied = std::move(*element);
					++copied;
				}
			}
			return copied;
		}

	private:

		using proper_predicate = typename detail::proper_value_function<Predicate, bool, element_type const &>::type;

		Input input;
		proper_predicate is_propagated;
	};

	template <class Input, class Predicate>
	auto make_filter_source(Input &&input, Predicate &&is_propagated)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> filter_source<
			typename std::decay<Input>::type,
			typename std::decay<Predicate>::type
		>
#endif
	{
		return filter_source<
				typename std::decay<Input>::type,
				typename std::decay<Predicate>::type
			>(
				std::forward<Input>(input),
				std::forward<Predicate>(is_propagated)
			);
	}
}

#endif
