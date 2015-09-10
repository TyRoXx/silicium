#ifndef SILICIUM_READY_FUTURE_HPP
#define SILICIUM_READY_FUTURE_HPP

#include <silicium/observable/observable.hpp>
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
		future_runtime_checked()
			: got(false)
		{
		}

		template <class Element>
		void begin_get(Element const &)
		{
			assert(!got);
			got = true;
		}

	private:

		bool got;
	};

	template <class Element, class CheckPolicy = future_unchecked>
	struct ready_future : private CheckPolicy
	{
		typedef Element element_type;

		ready_future()
		{
		}

#if !SILICIUM_COMPILER_GENERATES_MOVES
		ready_future(ready_future &&other)
			: value(std::move(other.value))
		{
		}

		ready_future &operator = (ready_future &&other)
		{
			value = std::move(other.value);
			return *this;
		}
#endif

		explicit ready_future(Element value)
			: value(std::move(value))
		{
		}

		template <class ElementObserver>
		void async_get_one(ElementObserver &&receiver)
		{
			CheckPolicy::begin_get(value);
			std::forward<ElementObserver>(receiver).got_element(std::move(value));
		}

	private:

		Element value;
	};

	template <class Element>
	auto make_ready_future_observable(Element &&value)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> ready_future<typename std::decay<Element>::type>
#endif
	{
		return ready_future<typename std::decay<Element>::type>(std::forward<Element>(value));
	}
}

#endif
