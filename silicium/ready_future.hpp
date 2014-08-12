#ifndef SILICIUM_READY_FUTURE_HPP
#define SILICIUM_READY_FUTURE_HPP

#include <silicium/observable.hpp>
#include <silicium/config.hpp>
#include <cassert>

namespace Si
{
	struct future_unchecked
	{
		template <class Element>
		void begin_get(Element const &)
		{
		}
	};

	struct future_runtime_checked
	{
		template <class Element>
		void begin_get(Element const &)
		{
			assert(!got);
			got = true;
		}

	private:

		bool got = false;
	};

	template <class Element, class CheckPolicy = future_unchecked>
	struct ready_future : private CheckPolicy
	{
		using element_type = Element;

		ready_future()
		{
		}

		explicit ready_future(Element value)
			: value(std::move(value))
		{
		}

		template <class ElementObserver>
		void async_get_one(ElementObserver &receiver)
		{
			CheckPolicy::begin_get(value);
			receiver.got_element(std::move(value));
		}

		void cancel()
		{
			SILICIUM_UNREACHABLE();
		}

	private:

		Element value;
	};

	template <class Element>
	auto make_ready_future(Element &&value) -> ready_future<typename std::decay<Element>::type>
	{
		return ready_future<typename std::decay<Element>::type>(std::forward<Element>(value));
	}
}

#endif
