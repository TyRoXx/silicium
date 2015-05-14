#ifndef SILICIUM_REACTIVE_COROUTINE_GENERATOR_HPP
#define SILICIUM_REACTIVE_COROUTINE_GENERATOR_HPP

#include <silicium/exchange.hpp>
#include <silicium/observable/yield_context.hpp>
#include <silicium/fast_variant.hpp>
#ifdef _WIN32
//win32.hpp will include Asio before coroutine will include Windows stuff
//because Asio wants to be first.
#	include <silicium/win32/win32.hpp>
#endif
#include <boost/coroutine/all.hpp>

namespace Si
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
			observable<nothing, ptr_observer<observer<nothing>>> *target;
		};

		template <class Element>
		struct make_command
		{
			typedef Si::fast_variant<result<Element *>, yield> type;
		};

		template <class Element>
		struct coroutine_yield_context_impl : detail::push_context_impl<Element>
		{
#if BOOST_VERSION >= 105500
			typedef typename boost::coroutines::coroutine<typename detail::make_command<Element>::type>::push_type consumer_type;
#else
			typedef typename boost::coroutines::coroutine<typename detail::make_command<Element>::type ()>::caller_type consumer_type;
#endif

			explicit coroutine_yield_context_impl(consumer_type &consumer)
				: consumer(&consumer)
			{
			}

			virtual void push_result(Element result) SILICIUM_OVERRIDE
			{
				(*consumer)(detail::result<Element *>(&result));
			}

			virtual void get_one(observable<nothing, ptr_observer<observer<nothing>>> &target) SILICIUM_OVERRIDE
			{
				(*consumer)(detail::yield{&target});
			}

		private:

			consumer_type *consumer;
		};
	}

	template <class Element>
	struct coroutine_generator_observable : private observer<nothing>
	{
		typedef Element element_type;

		coroutine_generator_observable()
			: receiver_(nullptr)
		{
		}

		coroutine_generator_observable(coroutine_generator_observable &&other)
			: receiver_(nullptr)
		{
			*this = std::move(other);
		}

		coroutine_generator_observable &operator = (coroutine_generator_observable &&other)
		{
			coro_ = std::move(other.coro_);
			action = std::move(other.action);
			receiver_ = std::move(other.receiver_);
			return *this;
		}

		template <class Action>
		explicit coroutine_generator_observable(Action &&action)
			: action(std::forward<Action>(action))
			, receiver_(nullptr)
		{
		}

		void async_get_one(ptr_observer<observer<element_type>> receiver)
		{
			receiver_ = receiver.get();
			next();
		}

	private:

		typedef typename detail::make_command<element_type>::type command_type;
		typedef
#if BOOST_VERSION >= 105500
			typename boost::coroutines::coroutine<command_type>::pull_type
#else
			boost::coroutines::coroutine<command_type ()>
#endif
		coroutine_type;
		typedef
#ifdef _MSC_VER
			std::shared_ptr<coroutine_type>
#else
			coroutine_type
#endif
			coroutine_holder;

		coroutine_holder coro_;
		std::function<void (push_context<Element> &)> action;
		Si::observer<Element> *receiver_;

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
			next();
		}

		void next()
		{
			if (action)
			{
				auto bound_action = action;
				coro_ =
#ifdef _MSC_VER
					std::make_shared<coroutine_type>
#else
					coroutine_type
#endif
						([bound_action](
#if BOOST_VERSION >= 105500
							typename boost::coroutines::coroutine<command_type>::push_type
#else
							typename coroutine_type::caller_type
#endif
							&push)
						{
							detail::coroutine_yield_context_impl<Element> yield_impl(push);
							push_context<Element> yield(yield_impl); //TODO: save this indirection
							return bound_action(yield);
						});
				action = nullptr;
			}
			else if (coro())
			{
				coro()();
			}
			if (coro())
			{
				command_type command = coro().get();
				return Si::visit<void>(
					command,
					[this](detail::result<element_type *> command)
					{
						return Si::exchange(receiver_, nullptr)->got_element(std::move(*command.value));
					},
					[this](detail::yield command)
					{
						command.target->async_get_one(observe_by_ref(static_cast<observer<nothing> &>(*this)));
					}
				);
			}
			else
			{
				Si::exchange(receiver_, nullptr)->ended();
			}
		}

		SILICIUM_DELETED_FUNCTION(coroutine_generator_observable(coroutine_generator_observable const &))
		SILICIUM_DELETED_FUNCTION(coroutine_generator_observable &operator = (coroutine_generator_observable const &))
	};

	template <class Element, class Action>
	auto make_coroutine_generator(Action &&action) -> coroutine_generator_observable<Element>
	{
		BOOST_STATIC_ASSERT_MSG(
			(std::is_same<void, decltype(action(std::declval<push_context<Element>&>()))>::value),
			"The return type of the action has to be void because we do not want to accidentally ignore a non-void result.");
		return coroutine_generator_observable<Element>(std::forward<Action>(action));
	}
}

#endif
