#ifndef SILICIUM_VIRTUALIZE_HPP
#define SILICIUM_VIRTUALIZE_HPP

#include <silicium/observable.hpp>
#include <silicium/override.hpp>

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

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			return next.async_get_one(receiver);
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			return next.cancel();
		}

	private:

		Observable next;
	};

	template <class Observable>
	auto virtualize(Observable &&next) -> virtualized_observable<typename std::decay<Observable>::type>
	{
		return virtualized_observable<typename std::decay<Observable>::type>(std::forward<Observable>(next));
	}
}

#endif
