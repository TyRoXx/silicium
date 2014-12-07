#ifndef SILICIUM_VIRTUALIZED_SINK_HPP
#define SILICIUM_VIRTUALIZED_SINK_HPP

#include <silicium/sink/ptr_sink.hpp>
#include <silicium/utility.hpp>

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

#if !SILICIUM_COMPILER_GENERATES_MOVES
		virtualized_sink(virtualized_sink &&other)
			: m_next(std::move(other.m_next))
		{
		}

		virtualized_sink &operator = (virtualized_sink &&other)
		{
			m_next = std::move(other.m_next);
			return *this;
		}
#endif

	private:

		Next m_next;
	};

	template <class Next>
	auto virtualize_sink(Next &&next)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> virtualized_sink<typename std::decay<Next>::type>
#endif
	{
		return virtualized_sink<typename std::decay<Next>::type>(std::forward<Next>(next));
	}

	template <class Next>
	auto erase_sink(Next &&next)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> ptr_sink<typename std::decay<Next>::type, std::unique_ptr<typename std::decay<Next>::type>>
#endif
	{
		typedef typename std::decay<Next>::type clean;
		return ptr_sink<clean, std::unique_ptr<clean>>(to_unique(virtualize_sink(std::forward<Next>(next))));
	}
}

#endif
