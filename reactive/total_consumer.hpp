#ifndef SILICIUM_REACTIVE_TOTAL_CONSUMER_HPP
#define SILICIUM_REACTIVE_TOTAL_CONSUMER_HPP

#include <reactive/observable.hpp>

namespace rx
{
	template <class Input>
	struct total_consumer : private rx::observer<typename Input::element_type>
	{
		typedef typename Input::element_type element_type;

		explicit total_consumer(Input input)
			: input(std::move(input))
		{
		}

		void start()
		{
			input.async_get_one(*this);
		}

	private:

		Input input;

		virtual void got_element(element_type value) SILICIUM_OVERRIDE
		{
			(void)value; //ignore result
			return start();
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
		}
	};

	template <class Input>
	auto make_total_consumer(Input &&input)
	{
		return total_consumer<typename std::decay<Input>::type>(std::forward<Input>(input));
	}
}

#endif
