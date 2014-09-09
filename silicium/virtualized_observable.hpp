#ifndef SILICIUM_VIRTUALIZED_OBSERVABLE_HPP
#define SILICIUM_VIRTUALIZED_OBSERVABLE_HPP

#include <silicium/config.hpp>
#include <silicium/observable.hpp>
#include <silicium/override.hpp>
#include <utility>
#include <boost/concept/requires.hpp>

namespace Si
{
	template <class Observable>
	struct virtualized_observable : observable<typename Observable::element_type>
	{
		using element_type = typename Observable::element_type;

		virtualized_observable()
		{
		}

		explicit virtualized_observable(Observable next)
			: next(std::move(next))
		{
		}

#if !SILICIUM_COMPILER_GENERATES_MOVES
		virtualized_observable(virtualized_observable &&other)
			: next(std::move(other.next))
		{
		}

		virtualized_observable &operator = (virtualized_observable &&other)
		{
			next = std::move(other.next);
			return *this;
		}
#endif

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			return next.async_get_one(receiver);
		}

	private:

		Observable next;
	};

	template <class Observable>
	BOOST_CONCEPT_REQUIRES(
		((Si::Observable<typename std::decay<Observable>::type>)),
		(virtualized_observable<typename std::decay<Observable>::type>))
	virtualize_observable(Observable &&next)
	{
		return virtualized_observable<typename std::decay<Observable>::type>(std::forward<Observable>(next));
	}
}

#endif
