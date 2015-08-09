#ifndef SILICIUM_COROUTINE_HPP
#define SILICIUM_COROUTINE_HPP

#include <silicium/config.hpp>
#include <silicium/observable/yield_context.hpp>

#define SILICIUM_HAS_COROUTINE_OBSERVABLE ((BOOST_VERSION >= 105300) && SILICIUM_HAS_EXCEPTIONS && SILICIUM_HAS_YIELD_CONTEXT && !SILICIUM_AVOID_BOOST_COROUTINE)

#if SILICIUM_HAS_COROUTINE_OBSERVABLE

#include <silicium/exchange.hpp>
#include <silicium/observable/observer.hpp>
#include <silicium/config.hpp>
#include <algorithm>
#include <boost/asio/io_service.hpp>
#include <boost/coroutine/all.hpp>

namespace Si
{
	namespace detail
	{
		struct coroutine_yield_context : basic_yield_context
		{
#if BOOST_VERSION >= 105500
			typedef boost::coroutines::coroutine<observable_type *>::push_type consumer_type;
#else
			typedef boost::coroutines::coroutine<observable_type *()>::caller_type consumer_type;
#endif

			explicit coroutine_yield_context(consumer_type &consumer)
				: consumer(&consumer)
			{
			}

			virtual void get_one(observable_type &target) SILICIUM_OVERRIDE
			{
				(*consumer)(&target);
			}

		private:

			consumer_type *consumer;
		};
	}

	template <class Element>
	struct coroutine_observable : private observer<nothing>
	{
		typedef Element element_type;

		coroutine_observable()
			: receiver_(nullptr)
		{
		}

		coroutine_observable(coroutine_observable &&other)
			: state(std::move(other.state))
			, action(std::move(other.action))
			, receiver_(nullptr)
		{
		}

		template <class Action>
		explicit coroutine_observable(Action &&action)
			: state()
			, action(std::forward<Action>(action))
			, receiver_(nullptr)
		{
		}

		coroutine_observable &operator = (coroutine_observable &&other)
		{
			state = std::move(other.state);
			action = std::move(other.action);
			receiver_ = std::move(other.receiver_);
			return *this;
		}

		void async_get_one(Si::ptr_observer<Si::observer<element_type>> receiver)
		{
			receiver_ = receiver.get();
			next();
		}

	private:

		typedef Observable<nothing, ptr_observer<observer<nothing>>>::interface *command_type;
		typedef
#if BOOST_VERSION >= 105500
			typename boost::coroutines::coroutine<command_type>::pull_type
#else
			boost::coroutines::coroutine<command_type ()>
#endif
		coroutine_type;
		typedef std::shared_ptr<coroutine_type> coroutine_holder;

		struct async_state
		{
			coroutine_holder coro_;
			bool has_finished;

			async_state()
				: has_finished(false)
			{
			}
		};

		std::shared_ptr<async_state> state;
		std::function<Element (yield_context)> action;
		Si::observer<Element> *receiver_;
		
		virtual void got_element(nothing) SILICIUM_OVERRIDE
		{
			next();
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			next();
		}

		void next()
		{
			auto keep_coro_alive = state;
			if (keep_coro_alive && keep_coro_alive->has_finished)
			{
				return Si::exchange(receiver_, nullptr)->ended();
			}
			if (action)
			{
				auto bound_action = Si::exchange(action, decltype(action)());
				auto new_state = std::make_shared<async_state>();
				state = new_state;
				keep_coro_alive = new_state;
				new_state->coro_ =
					std::make_shared<coroutine_type>
						([bound_action, this](
#if BOOST_VERSION >= 105500
							typename boost::coroutines::coroutine<command_type>::push_type
#else
							typename coroutine_type::caller_type
#endif
							&push)
						{
							detail::coroutine_yield_context yield_impl(push);
							yield_context yield(yield_impl); //TODO: save this indirection
							auto result = bound_action(yield);
							assert(!state->has_finished);
							state->has_finished = true;
							Si::exchange(receiver_, nullptr)->got_element(std::move(result));
						});
			}
			else if (*state->coro_)
			{
				(*state->coro_)();
			}
			if (keep_coro_alive->has_finished)
			{
				return;
			}
			if (!receiver_)
			{
				return;
			}
			if (*state->coro_)
			{
				command_type command = state->coro_->get();
				command->async_get_one(observe_by_ref(static_cast<observer<nothing> &>(*this)));
			}
			else
			{
				Si::exchange(receiver_, nullptr)->ended();
			}
		}

		SILICIUM_DELETED_FUNCTION(coroutine_observable(coroutine_observable const &))
		SILICIUM_DELETED_FUNCTION(coroutine_observable &operator = (coroutine_observable const &))
	};

	template <class Action>
	auto make_coroutine(Action &&action)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> coroutine_observable<decltype(std::declval<Action>()(std::declval<yield_context>()))>
#endif
	{
		return coroutine_observable<decltype(std::declval<Action>()(std::declval<yield_context>()))>(std::forward<Action>(action));
	}
}

#endif

#endif
