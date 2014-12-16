#ifndef SILICIUM_REACTIVE_TOTAL_CONSUMER_HPP
#define SILICIUM_REACTIVE_TOTAL_CONSUMER_HPP

#include <silicium/observable/observable.hpp>
#include <silicium/config.hpp>

namespace Si
{
	template <class Input>
	struct total_consumer : private observer<typename Input::element_type>
	{
		typedef typename Input::element_type element_type;

		total_consumer()
		{
		}

		explicit total_consumer(Input input)
			: input(std::move(input))
		{
		}

#if !SILICIUM_COMPILER_GENERATES_MOVES
		total_consumer(total_consumer &&other)
			: input(std::move(other.input))
		{
		}

		total_consumer &operator = (total_consumer &&other)
		{
			input = std::move(other.input);
			return *this;
		}
#endif

		void start()
		{
			input.async_get_one(observe_by_ref(static_cast<observer<typename Input::element_type> &>(*this)));
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
	auto make_total_consumer(Input &&input) -> total_consumer<typename std::decay<Input>::type>
	{
		return total_consumer<typename std::decay<Input>::type>(std::forward<Input>(input));
	}
}

#endif
