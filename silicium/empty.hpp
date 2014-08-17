#ifndef SILICIUM_EMPTY_HPP
#define SILICIUM_EMPTY_HPP

#include <silicium/override.hpp>
#include <silicium/observer.hpp>
#include <stdexcept>

namespace Si
{
	template <class Element>
	struct empty
	{
		void async_get_one(observer<Element> &receiver)
		{
			return receiver.ended();
		}

		void cancel()
		{
			throw std::logic_error("empty observable cannot be cancelled");
		}
	};
}

#endif
