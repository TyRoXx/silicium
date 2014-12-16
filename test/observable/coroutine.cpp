#include <silicium/observable/coroutine.hpp>
#include <silicium/observable/consume.hpp>
#include <silicium/observable/bridge.hpp>
#include <silicium/observable/total_consumer.hpp>
#include <silicium/observable/on_first.hpp>
#include <silicium/observable/for_each.hpp>
#include <silicium/observable/function_observer.hpp>
#include <silicium/asio/timer.hpp>
#include <silicium/to_unique.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/asio/io_service.hpp>
#include <functional>

BOOST_AUTO_TEST_CASE(coroutine_trivial)
{
	auto coro = Si::make_coroutine([](Si::yield_context)
	{
		return 2;
	});
	bool got_element = false;
	auto consumer_ = Si::consume<int>([&got_element](int i)
	{
		BOOST_CHECK_EQUAL(2, i);
		got_element = true;
	});
	coro.async_get_one(Si::observe_by_ref(consumer_));
	BOOST_CHECK(got_element);
}

BOOST_AUTO_TEST_CASE(coroutine_yield)
{
	Si::bridge<int> e;
	auto coro = Si::make_coroutine([&e](Si::yield_context yield)
	{
		return *yield.get_one(e) + 1;
	});
	bool got_element = false;
	auto consumer_ = Si::consume<int>([&got_element](int i)
	{
		BOOST_CHECK_EQUAL(5, i);
		got_element = true;
	});
	coro.async_get_one(Si::observe_by_ref(consumer_));
	BOOST_CHECK(!got_element);
	e.got_element(4);
	BOOST_CHECK(got_element);
}

BOOST_AUTO_TEST_CASE(coroutine_total_consumer)
{
	bool executed = false;
	auto consumer = Si::make_total_consumer(Si::make_coroutine([&executed](Si::yield_context)
	{
		executed = true;
		return Si::nothing();
	}));
	BOOST_CHECK(!executed);
	consumer.start();
	BOOST_CHECK(executed);
}

BOOST_AUTO_TEST_CASE(coroutine_self_destruct)
{
	std::size_t steps_done = 0;
	auto coro = Si::to_unique(Si::make_coroutine([&steps_done](Si::yield_context) -> Si::nothing
	{
		BOOST_REQUIRE_EQUAL(1u, steps_done);
		++steps_done;
		return {};
	}));
	BOOST_REQUIRE_EQUAL(0u, steps_done);
	auto handler = Si::on_first(Si::ref(*coro), [&coro, &steps_done](boost::optional<Si::nothing> value)
	{
		BOOST_CHECK(value);
		//this function is called in the coroutine
		BOOST_REQUIRE_EQUAL(2u, steps_done);
		++steps_done;
		//destroying the coroutine itself now should not crash or anything, it just works.
		coro.reset();
	});
	BOOST_REQUIRE_EQUAL(0u, steps_done);
	++steps_done;
	handler.start();
	BOOST_CHECK_EQUAL(3u, steps_done);
	BOOST_CHECK(!coro);
}

namespace Si
{
	namespace detail
	{
		template <class Function>
		auto lambda_to_value_impl(Function &&function, std::true_type)
		{
			return std::forward<Function>(function);
		}

		template <class Function, class Result, class Class, class ...Arguments>
		auto lambda_to_value_impl_lambda_case(Function &&function, Result (Class::*)(Arguments...) const)
		{
			return std::function<Result (Arguments...)>(std::forward<Function>(function));
		}

		template <class Function>
		auto lambda_to_value_impl(Function &&function, std::false_type)
		{
			typedef typename std::decay<Function>::type clean;
			return lambda_to_value_impl_lambda_case(std::forward<Function>(function), &clean::operator());
		}

		template <class Function>
		auto lambda_to_value(Function &&function)
		{
			typedef typename std::decay<Function>::type clean;
			return lambda_to_value_impl(
				std::forward<Function>(function),
				std::integral_constant<bool,
					std::is_move_assignable<clean>::value &&
					std::is_move_constructible<clean>::value
				>());
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

	private:

		Next m_next;
		Transformation m_transform;
	};

	namespace detail
	{
		template <class Element, class Next, class Transformation>
		auto transform_observer_impl(Next &&next, Transformation &&transform)
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
	{
		return detail::transform_observer_impl<Element>(std::forward<Next>(next), detail::lambda_to_value(std::forward<Transformation>(transform)));
	}

	struct spawn_context
	{
		typedef std::function<void (observable<nothing> &)> wait_function;

		spawn_context()
		{
		}

		explicit spawn_context(wait_function wait, std::weak_ptr<void> async_state)
			: m_wait(std::move(wait))
			, m_async_state(std::move(async_state))
		{
		}

		template <class Observable>
		boost::optional<typename std::decay<Observable>::type::element_type> get_one(Observable &&from)
		{
			typedef typename std::decay<Observable>::type::element_type element_type;
			boost::optional<element_type> result;
			auto waiting_for =
				virtualize_observable(
					transform_observer<nothing>(
						std::forward<Observable>(from),
						[this, &result](ptr_observer<observer<nothing>> previous_observer)
						{
							std::shared_ptr<void> async_state = this->m_async_state.lock();
							assert(async_state);
							assert(async_state.use_count() >= 2);
							return make_function_observer([previous_observer, async_state, &result](boost::optional<element_type> element)
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
							});
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
			{
			}

			template <class Function>
			void start(Function &&function)
			{
				m_coro = coroutine::pull_type([this, function](coroutine::push_type &push)
				{
					spawn_context context(
						[this, &push](observable<nothing> &waiting_for)
						{
							wait_for(waiting_for);
							if (m_is_still_waiting)
							{
								push(nothing());
								assert(!m_is_still_waiting);
							}
						},
						std::weak_ptr<void>(this->shared_from_this())
						);
					function(context);
				});
			}

		private:

			typedef boost::coroutines::asymmetric_coroutine<nothing> coroutine;

			coroutine::pull_type m_coro;
			bool m_is_still_waiting;

			void wait_for(observable<nothing> &waiting_for)
			{
				m_is_still_waiting = true;
				waiting_for.async_get_one(observe_by_ref(static_cast<observer<nothing> &>(*this)));
			}

			void got_element(nothing) SILICIUM_OVERRIDE
			{
				ended();
			}

			void ended() SILICIUM_OVERRIDE
			{
				m_is_still_waiting = false;
				m_coro();
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

BOOST_AUTO_TEST_CASE(coroutine_keep_self_alive)
{
	bool elapsed = false;
	boost::asio::io_service io;
	Si::spawn_coroutine([&io, &elapsed](Si::spawn_context yield)
	{
		BOOST_REQUIRE(!elapsed);
		auto timer = Si::asio::make_timer(io);
		timer.expires_from_now(std::chrono::microseconds(1));
		boost::optional<Si::asio::timer_elapsed> e = yield.get_one(Si::ref(timer));
		BOOST_CHECK(e);
		BOOST_REQUIRE(!elapsed);
		elapsed = true;
	});
	BOOST_CHECK(!elapsed);
	io.run();
	BOOST_CHECK(elapsed);
}
