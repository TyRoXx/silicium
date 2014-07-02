#ifndef SILICIUM_REACTIVE_FINITE_STATE_MACHINE_HPP
#define SILICIUM_REACTIVE_FINITE_STATE_MACHINE_HPP

#include <reactive/observable.hpp>

namespace rx
{
	template <class In, class State, class Step>
	struct finite_state_machine
			: public observable<State>
			, private observer<typename In::element_type>
	{
		typedef State element_type;

		finite_state_machine(In in, element_type initial_state, Step step)
			: in(std::move(in))
			, state(std::move(initial_state))
			, step(std::move(step))
		{
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			if (has_ended)
			{
				receiver.ended();
			}
			else
			{
				check_fetch();
				receiver.got_element(state);
			}
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
		}

	private:

		In in;
		element_type state;
		Step step;
		bool fetching = false;
		bool has_ended = false;

		virtual void got_element(typename In::element_type value) SILICIUM_OVERRIDE
		{
			assert(fetching);
			fetching = false;
			check_fetch();
			state = step(std::move(state), std::move(value));
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			assert(fetching);
			fetching = false;
			assert(!has_ended);
			has_ended = true;
		}

		void check_fetch()
		{
			if (!fetching)
			{
				fetching = true;
				in.async_get_one(*this);
			}
		}
	};

	template <class In, class State, class Step>
	auto make_finite_state_machine(In &&in, State &&initial_state, Step &&step)
	{
		return finite_state_machine<
				typename std::decay<In>::type,
				typename std::decay<State>::type,
				typename std::decay<Step>::type>(std::forward<In>(in), std::forward<State>(initial_state), std::forward<Step>(step));
	}
}

#endif
