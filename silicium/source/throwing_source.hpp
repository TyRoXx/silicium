#ifndef SILICIUM_THROWING_SOURCE_HPP
#define SILICIUM_THROWING_SOURCE_HPP

#include <silicium/source/source.hpp>

namespace Si
{
	template <class SourceOfErrorOrs>
	struct throwing_source
	{
		typedef typename SourceOfErrorOrs::element_type::value_type element_type;

		throwing_source()
		{
		}

		explicit throwing_source(SourceOfErrorOrs input)
			: m_input(std::move(input))
		{
		}

		iterator_range<element_type const *> map_next(std::size_t size)
		{
			return {};
		}

		element_type *copy_next(iterator_range<element_type *> destination)
		{
			auto copied = destination.begin();
			while (copied != destination.end())
			{
				auto element = Si::get(m_input);
				if (!element)
				{
					break;
				}
				*copied = std::move(element->get());
				++copied;
			}
			return copied;
		}

	private:

		SourceOfErrorOrs m_input;
	};

	template <class SourceOfErrorOrs>
	auto make_throwing_source(SourceOfErrorOrs &&input)
	{
		return throwing_source<typename std::decay<SourceOfErrorOrs>::type>(std::forward<SourceOfErrorOrs>(input));
	}
}

#endif
