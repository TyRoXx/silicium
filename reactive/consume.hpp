#ifndef SILICIUM_REACTIVE_CONSUME_HPP
#define SILICIUM_REACTIVE_CONSUME_HPP

#include <reactive/observable.hpp>

namespace rx
{
	template <class Element, class Consume>
	struct consumer : observer<Element>
	{
		typedef Element element_type;

		explicit consumer(Consume consume)
			: consume(std::move(consume))
		{
		}

		virtual void got_element(element_type value) SILICIUM_OVERRIDE
		{
			consume(std::move(value));
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
		}

	private:

		Consume consume;
	};

	template <class Element, class Consume>
	auto consume(Consume con)
	{
		return consumer<Element, typename std::decay<Consume>::type>(std::move(con));
	}
}

#endif
