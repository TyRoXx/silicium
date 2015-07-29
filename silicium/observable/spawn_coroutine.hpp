#ifndef SILICIUM_SPAWN_COROUTINE_HPP
#define SILICIUM_SPAWN_COROUTINE_HPP

#include <silicium/config.hpp>

#define SILICIUM_HAS_SPAWN_COROUTINE ((BOOST_VERSION >= 105300) && SILICIUM_HAS_EXCEPTIONS)

#if SILICIUM_HAS_SPAWN_COROUTINE

#include <silicium/observable/virtualized.hpp>
#include <silicium/observable/function_observer.hpp>
#include <silicium/config.hpp>
#include <silicium/optional.hpp>
#include <algorithm>
#include <boost/asio/io_service.hpp>
#include <boost/coroutine/all.hpp>

namespace Si
{
	namespace detail
	{
		template <class Function>
		auto lambda_to_value_impl(Function &&function, std::true_type)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			-> Function
#endif
		{
			return std::forward<Function>(function);
		}

		template <class Function, class Result, class Class, class ...Arguments>
		auto lambda_to_value_impl_lambda_case(Function &&function, Result(Class::*)(Arguments...) const)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			-> std::function<Result(Arguments...)>
#endif
		{
			return std::function<Result(Arguments...)>(std::forward<Function>(function));
		}

		template <class Function, class Clean = typename std::decay<Function>::type>
		auto lambda_to_value_impl(Function &&function, std::false_type)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			-> decltype(lambda_to_value_impl_lambda_case(std::forward<Function>(function), &Clean::operator()))
#endif
		{
			return lambda_to_value_impl_lambda_case(std::forward<Function>(function), &Clean::operator());
		}

		template <class Function>
		auto lambda_to_value(Function &&function)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			-> decltype(lambda_to_value_impl(
				std::forward<Function>(function),
				std::integral_constant<bool,
					Si::is_move_assignable<typename std::decay<Function>::type>::value &&
					Si::is_move_constructible<typename std::decay<Function>::type>::value
				>()
			))
#endif
		{
			typedef typename std::decay<Function>::type clean;
			return lambda_to_value_impl(
				std::forward<Function>(function),
				std::integral_constant<bool,
					Si::is_move_assignable<clean>::value &&
					Si::is_move_constructible<clean>::value
				>()
			);
		}
}

	template <class Element, class Next, class Transformation>
	struct observer_transforming
	{
		typedef Element element_type;

		observer_transforming()
		{
		}

		explicit observer_transforming(Next next, Transformation transform)
			: m_next(std::move(next))
			, m_transform(std::move(transform))
		{
		}

		template <class Observer>
		void async_get_one(Observer &&observer)
		{
			return m_next.async_get_one(m_transform(std::forward<Observer>(observer)));
		}

#if SILICIUM_COMPILER_GENERATES_MOVES
		observer_transforming(observer_transforming &&) = default;
		observer_transforming &operator = (observer_transforming &&) = default;
#else
		observer_transforming(observer_transforming &&other)
			: m_next(std::move(other.m_next))
			, m_transform(std::move(other.m_transform))
		{
		}
		
		observer_transforming &operator = (observer_transforming &&other)
		{
			m_next = std::move(other.m_next);
			m_transform = std::move(other.m_transform);
			return *this;
		}
#endif

	private:

		Next m_next;
		Transformation m_transform;
	};

	namespace detail
	{
		template <class Element, class Next, class Transformation>
		auto transform_observer_impl(Next &&next, Transformation &&transform)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			->observer_transforming<
			Element,
			typename std::decay<Next>::type,
			typename std::decay<Transformation>::type
			>
#endif
		{
			return observer_transforming<
				Element,
				typename std::decay<Next>::type,
				typename std::decay<Transformation>::type
			>(std::forward<Next>(next), std::forward<Transformation>(transform));
		}
	}

	template <class Element, class Next, class Transformation>
	auto transform_observer(Next &&next, Transformation &&transform)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> decltype(detail::transform_observer_impl<Element>(std::forward<Next>(next), detail::lambda_to_value(std::forward<Transformation>(transform))))
#endif
	{
		return detail::transform_observer_impl<Element>(std::forward<Next>(next), detail::lambda_to_value(std::forward<Transformation>(transform)));
	}

	struct spawn_context
	{
		typedef std::function<void(Observable<nothing, ptr_observer<observer<nothing>>>::interface &)> wait_function;

		spawn_context()
		{
		}

		explicit spawn_context(wait_function wait, std::weak_ptr<void> async_state)
			: m_wait(std::move(wait))
			, m_async_state(std::move(async_state))
		{
		}

		template <class Observable>
		Si::optional<typename std::decay<Observable>::type::element_type> get_one(Observable &&from)
		{
			typedef typename std::decay<Observable>::type::element_type element_type;
			Si::optional<element_type> result;
			auto waiting_for = virtualize_observable<ptr_observer<observer<nothing>>>(
				transform_observer<nothing>(
					std::forward<Observable>(from),
					[this, &result](ptr_observer<observer<nothing>> previous_observer)
					{
						std::shared_ptr<void> async_state = this->m_async_state.lock();
						assert(async_state);
						assert(async_state.use_count() >= 2);
						return make_function_observer(
#ifdef _MSC_VER
							std::function<void(Si::optional<element_type>)>
#endif
							([previous_observer, async_state, &result](Si::optional<element_type> element)
							{
								if (element)
								{
									result = std::move(*element);
									previous_observer.got_element(nothing());
								}
								else
								{
									previous_observer.ended();
								}
							})
						);
					}
				)
			);
			m_wait(waiting_for);
			return result;
		}

	private:

		wait_function m_wait;
		std::weak_ptr<void> m_async_state;
	};

	namespace detail
	{
		struct spawned : std::enable_shared_from_this<spawned>, private observer<nothing>
		{
			spawned()
			    : m_suspended(false)
			{
			}

			template <class Function>
			void start(Function &&function)
			{
				m_coro = coroutine([this, function](coroutine_push_type &push)
				{
					spawn_context context(
						[this, &push](Observable<nothing, ptr_observer<observer<nothing>>>::interface &waiting_for)
						{
							wait_for(waiting_for);
							if (m_waiting)
							{
								m_suspended = true;
								push(nothing());
							}
						},
						std::weak_ptr<void>(this->shared_from_this())
						);
					function(context);
				});
			}

		private:

#if BOOST_VERSION >= 105600
			typedef boost::coroutines::asymmetric_coroutine<nothing>::pull_type coroutine;
			typedef boost::coroutines::asymmetric_coroutine<nothing>::push_type coroutine_push_type;
#else
			typedef boost::coroutines::coroutine<nothing ()> coroutine;
			typedef boost::coroutines::coroutine<nothing ()>::caller_type coroutine_push_type;
#endif

			coroutine m_coro;
			bool m_waiting;
			bool m_suspended;

			void wait_for(Observable<nothing, ptr_observer<observer<nothing>>>::interface &waiting_for)
			{
				m_waiting = true;
				waiting_for.async_get_one(observe_by_ref(static_cast<observer<nothing> &>(*this)));
			}

			void got_element(nothing) SILICIUM_OVERRIDE
			{
				ended();
			}

			void ended() SILICIUM_OVERRIDE
			{
				m_waiting = false;
				if (m_suspended)
				{
					m_suspended = false;
					m_coro();
				}
			}
		};
	}

	template <class Function>
	void spawn_coroutine(Function &&function)
	{
		auto s = std::make_shared<detail::spawned>();
		s->start(std::forward<Function>(function));
	}
}

#endif

#endif
