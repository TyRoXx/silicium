#ifndef SILICIUM_REACTIVE_WHILE_HPP
#define SILICIUM_REACTIVE_WHILE_HPP

#include <reactive/observable.hpp>

namespace rx
{
	template <class Input, class ElementPredicate>
	struct while_observable
			: public observable<typename Input::element_type>
			, private observer<typename Input::element_type>
	{
		typedef typename Input::element_type element_type;

		while_observable(Input input, ElementPredicate is_not_end)
			: input(std::move(input))
			, is_not_end(std::move(is_not_end))
		{
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(!receiver_);
			receiver_ = &receiver;
			input.async_get_one(*this);
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			assert(receiver_);
			receiver_ = nullptr;
			return input.cancel();
		}

	private:

		Input input;
		ElementPredicate is_not_end;
		observer<element_type> *receiver_ = nullptr;

		virtual void got_element(typename Input::element_type value) SILICIUM_OVERRIDE
		{
			assert(receiver_);
			if (is_not_end(value))
			{
				exchange(receiver_, nullptr)->got_element(std::move(value));
			}
			else
			{
				exchange(receiver_, nullptr)->ended();
			}
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			assert(receiver_);
			exchange(receiver_, nullptr)->ended();
		}
	};

	template <class Input, class ElementPredicate>
	auto while_(Input &&input, ElementPredicate &&is_not_end)
	{
		return while_observable<
				typename std::decay<Input>::type,
				typename std::decay<ElementPredicate>::type>(std::forward<Input>(input), std::forward<ElementPredicate>(is_not_end));
	}
}

#endif
