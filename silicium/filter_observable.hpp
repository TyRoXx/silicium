#ifndef SILICIUM_FILTER_OBSERVABLE_HPP
#define SILICIUM_FILTER_OBSERVABLE_HPP

#include <silicium/observable.hpp>
#include <silicium/override.hpp>
#include <silicium/detail/proper_value_function.hpp>
#include <cassert>

namespace Si
{
	template <class Input, class Predicate>
	struct filter_observable
		: private observer<typename Input::element_type>
	{
		typedef typename Input::element_type element_type;

		filter_observable()
		{
		}

		filter_observable(Input input, Predicate is_propagated)
			: input(std::move(input))
			, is_propagated(std::move(is_propagated))
		{
		}

		void async_get_one(observer<element_type> &receiver)
		{
			assert(!receiver_);
			receiver_ = &receiver;
			input.async_get_one(*this);
		}

	private:

		using proper_predicate = typename detail::proper_value_function<Predicate, bool, element_type const &>::type;

		Input input;
		proper_predicate is_propagated;
		observer<element_type> *receiver_ = nullptr;

		virtual void got_element(element_type value) SILICIUM_OVERRIDE
		{
			assert(receiver_);
			if (!is_propagated(static_cast<typename std::add_const<element_type>::type &>(value)))
			{
				input.async_get_one(*this);
				return;
			}
			exchange(receiver_, nullptr)->got_element(std::move(value));
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			assert(receiver_);
			exchange(receiver_, nullptr)->ended();
		}
	};

	template <class Input, class Predicate>
	auto make_filter_observable(Input &&input, Predicate &&is_propagated) -> filter_observable<
		typename std::decay<Input>::type,
		typename std::decay<Predicate>::type>
	{
		return filter_observable<
			typename std::decay<Input>::type,
			typename std::decay<Predicate>::type>(std::forward<Input>(input), std::forward<Predicate>(is_propagated));
	}
}

#endif
