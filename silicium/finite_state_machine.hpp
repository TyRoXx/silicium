#ifndef SILICIUM_REACTIVE_FINITE_STATE_MACHINE_HPP
#define SILICIUM_REACTIVE_FINITE_STATE_MACHINE_HPP

#include <silicium/observable.hpp>
#include <silicium/override.hpp>
#include <cassert>
#include <boost/optional.hpp>

namespace Si
{
	template <class In, class State, class Step>
	struct finite_state_machine
			: public observable<State>
			, private observer<typename In::element_type>
	{
		typedef State element_type;

		finite_state_machine()
		{
		}

		finite_state_machine(In in, element_type initial_state, Step step)
			: in(std::move(in))
			, state(std::move(initial_state))
			, step(std::move(step))
		{
		}

#ifdef _MSC_VER
		finite_state_machine(finite_state_machine &&other)
		{
			*this = std::move(other);
		}

		finite_state_machine &operator = (finite_state_machine &&other)
		{
			//TODO: exception safety
			in = std::move(other.in);
			state = std::move(other.state);
			step = std::move(other.step);
			fetching = std::move(other.fetching);
			has_ended = std::move(other.has_ended);
			receiver_ = std::move(other.receiver_);
			return *this;
		}
#else
		finite_state_machine(finite_state_machine &&) = default;
		finite_state_machine &operator = (finite_state_machine &&) = default;
#endif

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(!receiver_);
			receiver_ = &receiver;
			in.async_get_one(*this);
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			//TODO
		}

	private:

		In in;
		element_type state;
		Step step;
		bool fetching = false;
		bool has_ended = false;
		observer<element_type> *receiver_ = nullptr;

		virtual void got_element(typename In::element_type value) SILICIUM_OVERRIDE
		{
			assert(receiver_);
			boost::optional<element_type> next = step(std::move(state), std::move(value));
			if (next)
			{
				state = std::move(*next);
				exchange(receiver_, nullptr)->got_element(state);
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

		void check_fetch()
		{
			if (!fetching)
			{
				fetching = true;
				in.async_get_one(*this);
			}
		}

		BOOST_DELETED_FUNCTION(finite_state_machine(finite_state_machine const &))
		BOOST_DELETED_FUNCTION(finite_state_machine &operator = (finite_state_machine const &))
	};

	template <class In, class State, class Step>
	auto make_finite_state_machine(In &&in, State &&initial_state, Step &&step) -> finite_state_machine<
		typename std::decay<In>::type,
		typename std::decay<State>::type,
		typename std::decay<Step>::type>
	{
		return finite_state_machine<
				typename std::decay<In>::type,
				typename std::decay<State>::type,
				typename std::decay<Step>::type>(std::forward<In>(in), std::forward<State>(initial_state), std::forward<Step>(step));
	}
}

#endif
