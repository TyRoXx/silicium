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

			result()
			{
			}

			explicit result(Element value)
				: value(std::move(value))
			{
			}

#ifdef _MSC_VER
			result(result &&other)
				: value(std::move(other.value))
			{
			}

			result &operator = (result &&other)
			{
				value = std::move(other.value);
				return *this;
			}

			result(result const &other)
				: value(other.value)
			{
			}

			result &operator = (result const &other)
			{
				value = other.value;
				return *this;
			}
#endif
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

	template <class Element = nothing>
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
				return nothing{};
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
			, private rx::observer<nothing>
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
				yield_context<Element> yield(push);
				return action(yield);
			})
#ifdef _MSC_VER
			)
#endif
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
		typedef typename boost::coroutines::coroutine<command_type>::pull_type coroutine_type;
		using coroutine_holder =
#ifdef _MSC_VER
			std::shared_ptr<coroutine_type>
#else
			coroutine_type
#endif
			;

		coroutine_holder coro_;
		rx::observer<Element> *receiver_ = nullptr;
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
			rx::exchange(receiver_, nullptr)->ended();
		}

		void next()
		{
			if (!rx::exchange(first, false))
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
