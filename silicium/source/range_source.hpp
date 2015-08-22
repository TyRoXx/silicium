#ifndef SILICIUM_RANGE_SOURCE_HPP
#define SILICIUM_RANGE_SOURCE_HPP

#include <silicium/source/source.hpp>
#include <boost/range/value_type.hpp>
#include <boost/concept_check.hpp>

namespace Si
{
	template <class ForwardRange>
	struct range_source
	{
		typedef typename boost::range_value<ForwardRange>::type element_type;

		range_source()
		{
		}

		explicit range_source(ForwardRange range)
			: m_range(std::move(range))
		{
		}

		ForwardRange &range()
		{
			return m_range;
		}

		ForwardRange const &range() const
		{
			return m_range;
		}

		iterator_range<element_type const *> map_next(std::size_t size)
		{
			return map_next_impl(size, std::is_pointer<iterator>());
		}

		element_type *copy_next(iterator_range<element_type *> destination)
		{
			auto copied = destination.begin();
			while (!m_range.empty() && (copied != destination.end()))
			{
				*copied = std::move(m_range.front());
				++copied;
				m_range.pop_front();
			}
			return copied;
		}

	private:

		typedef typename boost::range_iterator<ForwardRange>::type iterator;

		ForwardRange m_range;

		iterator_range<element_type const *> map_next_impl(std::size_t size, std::true_type)
		{
			boost::ignore_unused_variable_warning(size);
			if (m_range.empty())
			{
				return {};
			}
			element_type const * const data = &m_range.front();
			return make_iterator_range(data, data + m_range.size());
		}

		iterator_range<element_type const *> map_next_impl(std::size_t size, std::false_type)
		{
			boost::ignore_unused_variable_warning(size);
			return {};
		}
	};

	template <class ForwardRange>
	auto make_range_source(ForwardRange &&range)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> range_source<typename std::decay<ForwardRange>::type>
#endif
	{
		return range_source<typename std::decay<ForwardRange>::type>(std::forward<ForwardRange>(range));
	}
}

#endif
