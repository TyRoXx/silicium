#ifndef SILICIUM_LIMITED_OBSERVABLE_HPP
#define SILICIUM_LIMITED_OBSERVABLE_HPP

#include <silicium/config.hpp>
#include <utility>

namespace Si
{
	inline bool decrement_once(unsigned long long &remaining_count)
	{
		if (remaining_count == 0)
		{
			return false;
		}
		--remaining_count;
		return true;
	}

	template <class Next, class Limit>
	struct limited_observable
	{
		typedef typename Next::element_type element_type;

		explicit limited_observable(Next next, Limit limit)
		    : m_next(std::move(next))
		    , m_remaining_limit(std::move(limit))
		{
		}

		template <class Observer>
		void async_get_one(Observer &&observer)
		{
			if (decrement_once(m_remaining_limit))
			{
				m_next.async_get_one(std::forward<Observer>(observer));
			}
			else
			{
				std::forward<Observer>(observer).ended();
			}
		}

#if SILICIUM_COMPILER_GENERATES_MOVES
		limited_observable(limited_observable &&) = default;
		limited_observable &operator=(limited_observable &&) = default;
#else
		limited_observable(limited_observable &&other)
		    : m_next(std::move(other.m_next))
		    , m_remaining_limit(std::move(other.m_remaining_limit))
		{
		}

		limited_observable &operator=(limited_observable &&other)
		{
			m_next = std::move(other.m_next);
			m_remaining_limit = std::move(other.m_remaining_limit);
			return *this;
		}
#endif

	private:
		Next m_next;
		Limit m_remaining_limit;
	};

	template <class Next, class Limit>
	auto make_limited_observable(Next &&next, Limit &&limit)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
	    -> limited_observable<typename std::decay<Next>::type,
	                          typename std::decay<Limit>::type>
#endif
	{
		return limited_observable<typename std::decay<Next>::type,
		                          typename std::decay<Limit>::type>(
		    std::forward<Next>(next), std::forward<Limit>(limit));
	}
}

#endif
