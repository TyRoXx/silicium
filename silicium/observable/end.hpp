#ifndef SILICIUM_END_OBSERVABLE_HPP
#define SILICIUM_END_OBSERVABLE_HPP

#include <silicium/observable.hpp>
#include <silicium/exchange.hpp>
#include <silicium/config.hpp>
#include <silicium/override.hpp>
#include <cassert>

namespace Si
{
	struct ended
	{
	};

	template <class Input>
	struct end_observable : private observer<typename Input::element_type>
	{
		using element_type = Si::ended;

		end_observable()
		{
		}

		explicit end_observable(Input input)
			: input(std::move(input))
		{
		}

		void async_get_one(observer<element_type> &receiver)
		{
			assert(!receiver_);
			if (has_ended)
			{
				return receiver.ended();
			}
			receiver_ = &receiver;
			next();
		}

	private:

		Input input;
		observer<element_type> *receiver_ = nullptr;
		bool has_ended = false;

		void next()
		{
			assert(receiver_);
			input.async_get_one(*this);
		}

		virtual void got_element(typename Input::element_type value) SILICIUM_OVERRIDE
		{
			boost::ignore_unused_variable_warning(value);
			next();
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			has_ended = true;
			Si::exchange(receiver_, nullptr)->got_element(element_type());
		}
	};

	template <class Input>
	auto make_end_observable(Input &&input)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> end_observable<typename std::decay<Input>::type>
#endif
	{
		return end_observable<typename std::decay<Input>::type>(std::forward<Input>(input));
	}
}

#endif
