#ifndef SILICIUM_REACTIVE_COROUTINE_HPP
#define SILICIUM_REACTIVE_COROUTINE_HPP

#include <silicium/transform.hpp>
#include <silicium/config.hpp>
#include <silicium/ref.hpp>
#include <silicium/exchange.hpp>
#include <silicium/yield_context.hpp>
#include <boost/coroutine/all.hpp>

namespace Si
{
	namespace detail
	{
		template <class Element>
		struct coroutine_yield_context_impl : detail::yield_context_impl<Element>
		{
			typedef typename boost::coroutines::coroutine<typename detail::make_command<Element>::type>::push_type consumer_type;

			explicit coroutine_yield_context_impl(consumer_type &consumer)
				: consumer(&consumer)
			{
			}

			virtual void push_result(Element result) SILICIUM_OVERRIDE
			{
				(*consumer)(detail::result<Element>(std::move(result)));
			}

			virtual void get_one(observable<nothing> &target) SILICIUM_OVERRIDE
			{
				(*consumer)(detail::yield{&target});
			}

		private:

			consumer_type *consumer = nullptr;
		};
	}

	template <class Element>
	struct coroutine_observable
		: private Si::observer<nothing>
		, public boost::static_visitor<> //TODO make private
	{
		typedef Element element_type;

		coroutine_observable()
		{
		}

		coroutine_observable(coroutine_observable &&other)
		{
			*this = std::move(other);
		}

		coroutine_observable &operator = (coroutine_observable &&other)
		{
			coro_ = std::move(other.coro_);
			receiver_ = std::move(other.receiver_);
			first = std::move(other.first);
			return *this;
		}

		template <class Action>
		explicit coroutine_observable(Action action)
			: coro_(
#ifdef _MSC_VER
			std::make_shared<coroutine_type>(
#endif
			[action](typename boost::coroutines::coroutine<command_type>::push_type &push)
			{
				detail::coroutine_yield_context_impl<Element> yield_impl(push);
				yield_context<Element> yield(yield_impl); //TODO: save this indirection
				return action(yield);
			})
#ifdef _MSC_VER
			)
#endif
		{
		}

		void async_get_one(Si::observer<element_type> &receiver)
		{
			receiver_ = &receiver;
			next();
		}

		void cancel()
		{
			throw std::logic_error("not implemented");
		}

		//TODO make private
		void operator()(detail::result<element_type> command)
		{
			return Si::exchange(receiver_, nullptr)->got_element(std::move(command.value));
		}

		//TODO make private
		void operator()(detail::yield command)
		{
			command.target->async_get_one(*this);
		}

	private:

		typedef typename detail::make_command<element_type>::type command_type;
		typedef typename boost::coroutines::coroutine<command_type>::pull_type coroutine_type;
		using coroutine_holder =
#ifdef _MSC_VER
			std::shared_ptr<coroutine_type>
#else
			coroutine_type
#endif
			;

		coroutine_holder coro_;
		Si::observer<Element> *receiver_ = nullptr;
		bool first = true;

		coroutine_type &coro()
		{
			return
#ifdef _MSC_VER
				*
#endif
				coro_;
		}

		virtual void got_element(nothing) SILICIUM_OVERRIDE
		{
			next();
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			Si::exchange(receiver_, nullptr)->ended();
		}

		void next()
		{
			if (!Si::exchange(first, false))
			{
				coro()();
			}
			if (coro())
			{
				command_type command = coro().get();
				return Si::apply_visitor(*this, command);
			}
			else
			{
				Si::exchange(receiver_, nullptr)->ended();
			}
		}
	};

	template <class Element, class Action>
	auto make_coroutine(Action &&action) -> coroutine_observable<Element>
	{
		return coroutine_observable<Element>(std::forward<Action>(action));
	}
}

#endif
