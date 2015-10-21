#ifndef SILICIUM_MULTI_SINK_HPP
#define SILICIUM_MULTI_SINK_HPP

#include <silicium/sink/append.hpp>

namespace Si
{
	template <class Element, class Error, class GetChildren>
	struct multi_sink
	{
		typedef Element element_type;
		typedef Error error_type;

		multi_sink()
		{
		}

		explicit multi_sink(GetChildren get_children)
		    : m_get_children(std::move(get_children))
		{
		}

		error_type append(iterator_range<element_type const *> data) const
		{
			for (auto &&child_ptr : m_get_children())
			{
				auto error = Si::append(*child_ptr, data);
				if (error)
				{
					return error;
				}
			}
			return error_type();
		}

	private:
		GetChildren m_get_children;
	};

	template <class Element, class Error, class GetChildren>
	auto make_multi_sink(GetChildren &&get_children)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
	    -> multi_sink<Element, Error, typename std::decay<GetChildren>::type>
#endif
	{
		return multi_sink<Element, Error, typename std::decay<GetChildren>::type>(
		    std::forward<GetChildren>(get_children));
	}
}

#endif
