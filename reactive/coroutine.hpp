#ifndef SILICIUM_REACTIVE_COROUTINE_HPP
#define SILICIUM_REACTIVE_COROUTINE_HPP

#include <reactive/transform.hpp>
#include <reactive/config.hpp>
#include <reactive/ref.hpp>
#include <reactive/exchange.hpp>
#include <boost/coroutine/all.hpp>
#include <silicium/fast_variant.hpp>

namespace rx
{
	namespace detail
	{
		template <class Element>
		struct result
		{
			Element value;
		};

		struct yield
		{
			rx::observable<nothing> *target;
		};

		template <class Element>
		struct make_command
		{
			typedef Si::fast_variant<result<Element>, yield> type;
		};
	}

	template <class Element>
	struct yield_context
	{
		typedef typename boost::coroutines::coroutine<typename detail::make_command<Element>::type>::push_type consumer_type;

		explicit yield_context(consumer_type &consumer)
			: consumer(&consumer)
		{
		}

		void operator()(Element result)
		{
			(*consumer)(detail::result<Element>{std::move(result)});
		}

		template <class Gotten>
		boost::optional<Gotten> get_one(rx::observable<Gotten> &from)
		{
			boost::optional<Gotten> result;
			auto tf = rx::transform(rx::ref(from), [&result](Gotten element)
			{
				assert(!result);
				result = std::move(element);
				return detail::nothing{};
			});
			(*consumer)(detail::yield{&tf});
			return result;
		}

	private:

		consumer_type *consumer;
	};

	template <class Element>
	struct coroutine_observable
			: public rx::observable<Element>
			, private rx::observer<detail::nothing>
			, public boost::static_visitor<> //TODO make private
	{
		typedef Element element_type;

		template <class Action>
		explicit coroutine_observable(Action action)
			: coro([action](typename boost::coroutines::coroutine<command_type>::push_type &push)
			{
				yield_context<Element> yield(push);
				return action(yield);
			})
		{
		}

		virtual void async_get_one(rx::observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			receiver_ = &receiver;
			next();
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			throw std::logic_error("not implemented");
		}

		//TODO make private
		void operator()(detail::result<element_type> command)
		{
			return rx::exchange(receiver_, nullptr)->got_element(std::move(command.value));
		}

		//TODO make private
		void operator()(detail::yield command)
		{
			command.target->async_get_one(*this);
		}

	private:

		typedef typename detail::make_command<element_type>::type command_type;

		typename boost::coroutines::coroutine<command_type>::pull_type coro;
		rx::observer<Element> *receiver_ = nullptr;
		bool first = true;

		virtual void got_element(detail::nothing) SILICIUM_OVERRIDE
		{
			next();
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			exchange(receiver_, nullptr)->ended();
		}

		void next()
		{
			if (!rx::exchange(first, false))
			{
				coro();
			}
			if (coro)
			{
				command_type command = coro.get();
				return Si::apply_visitor(*this, command);
			}
			else
			{
				rx::exchange(receiver_, nullptr)->ended();
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
