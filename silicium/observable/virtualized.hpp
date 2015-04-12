#ifndef SILICIUM_VIRTUALIZED_OBSERVABLE_HPP
#define SILICIUM_VIRTUALIZED_OBSERVABLE_HPP

#include <silicium/config.hpp>
#include <silicium/observable/observable.hpp>
#include <utility>

namespace Si
{
	template <class Observable, class Observer>
	struct virtualized_observable : observable<typename Observable::element_type, Observer>
	{
		typedef typename Observable::element_type element_type;

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

		virtual void async_get_one(Observer receiver) SILICIUM_OVERRIDE
		{
			return next.async_get_one(receiver);
		}

	private:

		Observable next;
	};

	template <class Observer, class Observable>
	virtualized_observable<typename std::decay<Observable>::type, Observer>
	virtualize_observable(Observable &&next)
	{
		return virtualized_observable<typename std::decay<Observable>::type, Observer>(std::forward<Observable>(next));
	}
}

#endif
