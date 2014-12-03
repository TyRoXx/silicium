#ifndef SILICIUM_VIRTUALIZED_SINK_HPP
#define SILICIUM_VIRTUALIZED_SINK_HPP

#include <silicium/sink/sink.hpp>

namespace Si
{
	template <class Next>
	struct virtualized_sink : sink<typename Next::element_type, typename Next::error_type>
	{
		typedef typename Next::element_type element_type;
		typedef typename Next::error_type error_type;

		virtualized_sink()
		{
		}

		explicit virtualized_sink(Next next)
			: m_next(std::move(next))
		{
		}

		virtual error_type append(iterator_range<element_type const *> data) SILICIUM_OVERRIDE
		{
			return m_next.append(data);
		}

	private:

		Next m_next;
	};

	template <class Next>
	auto virtualize_sink(Next &&next)
	{
		return virtualized_sink<typename std::decay<Next>::type>(std::forward<Next>(next));
	}
}

#endif
