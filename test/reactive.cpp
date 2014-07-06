#include <reactive/buffer.hpp>
#include <reactive/generate.hpp>
#include <reactive/consume.hpp>
#include <reactive/tuple.hpp>
#include <reactive/ptr_observable.hpp>
#include <reactive/transform.hpp>
#include <reactive/bridge.hpp>
#include <reactive/take.hpp>
#include <reactive/enumerate.hpp>
#include <reactive/cache.hpp>
#include <reactive/connector.hpp>
#include <reactive/receiver.hpp>
#include <reactive/deref_optional.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/coroutine/all.hpp>

namespace Si
{
	BOOST_AUTO_TEST_CASE(reactive_take)
	{
		auto zeros = rx::generate([]{ return 0; });
		auto ones  = rx::generate([]{ return 1; });
		auto both = rx::make_tuple(zeros, ones);
		std::vector<std::tuple<int, int>> const expected(4, std::make_tuple(0, 1));
		std::vector<std::tuple<int, int>> const generated = rx::take(both, expected.size());
		BOOST_CHECK(expected == generated);
	}

	BOOST_AUTO_TEST_CASE(reactive_transform)
	{
		auto twos = rx::generate([]{ return 2; });
		auto ones  = rx::generate([]{ return 1; });
		auto both = rx::make_tuple(twos, ones);
		auto added = rx::transform(both, [](std::tuple<int, int> const &element)
		{
			return std::get<0>(element) + std::get<1>(element);
		});
		std::vector<int> const expected(4, 3);
		std::vector<int> const generated = rx::take(added, expected.size());
		BOOST_CHECK(expected == generated);
	}

	BOOST_AUTO_TEST_CASE(reactive_bridge)
	{
		auto bridge = std::make_shared<rx::bridge<int>>();
		rx::ptr_observable<int, std::shared_ptr<rx::observable<int>>> first(bridge);
		auto ones  = rx::generate([]{ return 1; });
		auto both = rx::make_tuple(first, ones);
		auto added = rx::transform(both, [](std::tuple<int, int> const &element)
		{
			return std::get<0>(element) + std::get<1>(element);
		});
		std::vector<int> generated;
		auto consumer = rx::consume<int>([&generated](int element)
		{
			generated.emplace_back(element);
		});
		BOOST_CHECK(generated.empty());

		added.async_get_one(consumer);
		BOOST_CHECK(generated.empty());

		bridge->got_element(2);
		std::vector<int> const expected(1, 3);
		BOOST_CHECK(expected == generated);
	}

	BOOST_AUTO_TEST_CASE(reactive_make_buffer)
	{
		auto bridge = std::make_shared<rx::bridge<int>>();
		rx::ptr_observable<int, std::shared_ptr<rx::observable<int>>> first{bridge};
		auto buf = rx::make_buffer(first, 2);

		std::vector<int> generated;
		auto consumer = rx::consume<int>([&generated](int element)
		{
			generated.emplace_back(element);
		});
		BOOST_CHECK(generated.empty());

		for (size_t i = 0; i < 2; ++i)
		{
			BOOST_REQUIRE(bridge->is_waiting());
			bridge->got_element(7);
		}
		BOOST_CHECK(!bridge->is_waiting());
		BOOST_CHECK(generated.empty());

		buf.async_get_one(consumer);
		std::vector<int> expected(1, 7);
		BOOST_CHECK(expected == generated);

		buf.async_get_one(consumer);
		expected.emplace_back(7);
		BOOST_CHECK(expected == generated);

		buf.async_get_one(consumer);
		BOOST_CHECK(expected == generated);
	}

	namespace detail
	{
		template <class Element>
		struct result
		{
			Element value;
		};

		struct nothing
		{
		};

		struct yield
		{
			rx::observable<nothing> *target;
		};

		template <class Element>
		struct make_command
		{
			typedef boost::variant<result<Element>, yield> type;
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
				return boost::apply_visitor(*this, command);
			}
			else
			{
				rx::exchange(receiver_, nullptr)->ended();
			}
		}
	};

	template <class Element, class Action>
	auto make_coroutine(Action &&action)
	{
		return coroutine_observable<Element>(std::forward<Action>(action));
	}

	BOOST_AUTO_TEST_CASE(reactive_coroutine_generate)
	{
		auto co = make_coroutine<int>([](yield_context<int> &yield) -> void
		{
			yield(1);
			yield(2);
		});
		std::vector<int> generated;
		auto collector = rx::consume<int>([&generated](int element)
		{
			generated.emplace_back(element);
		});
		for (;;)
		{
			auto old_size = generated.size();
			co.async_get_one(collector);
			if (generated.size() == old_size)
			{
				break;
			}
			BOOST_REQUIRE(generated.size() == old_size + 1);
		}
		std::vector<int> const expected{1, 2};
		BOOST_CHECK(expected == generated);
	}

	BOOST_AUTO_TEST_CASE(reactive_coroutine_get_one)
	{
		rx::bridge<int> asyncs;
		auto co = make_coroutine<int>([&asyncs](yield_context<int> &yield) -> void
		{
			auto a = yield.get_one(asyncs);
			BOOST_REQUIRE(a);
			yield(*a - 1);
		});
		std::vector<int> generated;
		auto collector = rx::consume<int>([&generated](int element)
		{
			generated.emplace_back(element);
		});
		co.async_get_one(collector);
		BOOST_REQUIRE(generated.empty());
		asyncs.got_element(4);
		std::vector<int> const expected{3};
		BOOST_CHECK(expected == generated);
	}
}
