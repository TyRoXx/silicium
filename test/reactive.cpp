#include <silicium/buffer.hpp>
#include <silicium/variant_observable.hpp>
#include <silicium/coroutine.hpp>
#include <silicium/generator_observable.hpp>
#include <silicium/consume.hpp>
#include <silicium/timer.hpp>
#include <silicium/tuple.hpp>
#include <silicium/total_consumer.hpp>
#include <silicium/ptr_observable.hpp>
#include <silicium/transform.hpp>
#include <silicium/bridge.hpp>
#include <silicium/enumerate.hpp>
#include <silicium/cache.hpp>
#include <silicium/empty.hpp>
#include <silicium/thread.hpp>
#include <silicium/boost_threading.hpp>
#include <silicium/std_threading.hpp>
#include <silicium/deref_optional.hpp>
#include <silicium/to_unique.hpp>
#include <boost/container/string.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/thread.hpp>
#include <boost/mpl/list.hpp>
#include <unordered_map>
#include <future>

namespace Si
{
#if SILICIUM_RX_TUPLE_AVAILABLE
	BOOST_AUTO_TEST_CASE(reactive_transform)
	{
		auto twos = Si::make_generator_observable([]{ return 2; });
		auto ones  = Si::make_generator_observable([]{ return 1; });
		auto both = Si::make_tuple(twos, ones);
		auto added = Si::transform(both, [](std::tuple<int, int> const &element)
		{
			return std::get<0>(element) + std::get<1>(element);
		});
		std::vector<int> generated;
		auto consumer = Si::consume<int>([&generated](int element)
		{
			generated.emplace_back(element);
		});
		added.async_get_one(consumer);
		std::vector<int> const expected(1, 3);
		BOOST_CHECK(expected == generated);
	}

	template <class Observable>
	void test_value_semantics(Observable &&original)
	{
		auto move_constructed = std::move(original);
		original = std::move(move_constructed);
		typename std::decay<decltype(original)>::type default_constructed;
	}

	BOOST_AUTO_TEST_CASE(transform_value_semantics)
	{
		int dummy;
		test_value_semantics(Si::transform(Si::empty<int>(), [&dummy](int) { return 0; }));
	}
#endif

#if SILICIUM_RX_TUPLE_AVAILABLE
	BOOST_AUTO_TEST_CASE(reactive_bridge)
	{
		auto bridge = std::make_shared<Si::bridge<int>>();
		Si::ptr_observable<int, std::shared_ptr<Si::observable<int>>> first(bridge);
		auto ones  = Si::make_generator_observable([]{ return 1; });
		auto both = Si::make_tuple(first, ones);
		auto added = Si::transform(both, [](std::tuple<int, int> const &element)
		{
			return std::get<0>(element) + std::get<1>(element);
		});
		std::vector<int> generated;
		auto consumer = Si::consume<int>([&generated](int element)
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
#endif

	BOOST_AUTO_TEST_CASE(reactive_make_buffer)
	{
		auto bridge = std::make_shared<Si::bridge<int>>();
		Si::ptr_observable<int, std::shared_ptr<Si::observable<int>>> first{bridge};
		auto buf = Si::make_buffer(first, 2);
		buf.prefetch();

		std::vector<int> generated;
		auto consumer = Si::consume<int>([&generated](int element)
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

	BOOST_AUTO_TEST_CASE(reactive_coroutine_generate)
	{
		auto co = Si::make_coroutine<int>([](Si::yield_context<int> &yield) -> void
		{
			yield(1);
			yield(2);
		});
		std::vector<int> generated;
		auto collector = Si::consume<int>([&generated](int element)
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
		Si::bridge<int> asyncs;
		bool exited_cleanly = false;
		auto co = Si::make_coroutine<int>([&asyncs, &exited_cleanly](Si::yield_context<int> &yield) -> void
		{
			auto a = yield.get_one(asyncs);
			BOOST_REQUIRE(a);
			yield(*a - 1);
			exited_cleanly = true;
		});
		std::vector<int> generated;
		auto collector = Si::consume<int>([&generated](int element)
		{
			generated.emplace_back(element);
		});
		co.async_get_one(collector);
		BOOST_REQUIRE(generated.empty());
		asyncs.got_element(4);

		//TODO: reading past the end should not be the required way to avoid a force unwind of the coroutine
		//      because the unwinding is done by throwing an exception.
		co.async_get_one(collector);
		BOOST_CHECK(exited_cleanly);

		std::vector<int> const expected{3};
		BOOST_CHECK(expected == generated);
	}

#if SILICIUM_RX_VARIANT_AVAILABLE
	BOOST_AUTO_TEST_CASE(reactive_make_variant)
	{
		Si::bridge<int> first;
		using string =
#ifdef _MSC_VER //does not compile with boost::container::string in VC++ 2013
			std::string
#else
			boost::container::string
#endif
			;
		Si::bridge<string> second;
		auto variants = make_variant(Si::ref(first), Si::ref(second));

		typedef Si::fast_variant<int, string> variant;
		std::vector<variant> produced;
		auto consumer = Si::consume<variant>([&produced](variant element)
		{
			produced.emplace_back(std::move(element));
		});

		variants.async_get_one(consumer);
		BOOST_CHECK(produced.empty());
		first.got_element(4);

		variants.async_get_one(consumer);
		BOOST_CHECK_EQUAL(1U, produced.size());
		second.got_element("Hi");

		std::vector<variant> const expected
		{
			4,
			string("Hi")
		};

		BOOST_CHECK(expected == produced);
	}
#endif

	template <class Element, class Action>
	struct blocking_then_state : Si::observer<Element>
	{
		boost::asio::io_service *dispatcher;
		boost::optional<boost::asio::io_service::work> blocker;
		Action action;
		Si::observable<Element> *from = nullptr;

		explicit blocking_then_state(boost::asio::io_service &dispatcher, Action action)
			: dispatcher(&dispatcher)
			, blocker(boost::in_place(boost::ref(dispatcher)))
			, action(std::move(action))
		{
		}

		~blocking_then_state()
		{
			if (!from)
			{
				return;
			}
			from->cancel();
		}

		virtual void got_element(Element value) SILICIUM_OVERRIDE
		{
			assert(dispatcher);
			dispatcher->post(boost::bind<void>(action, boost::make_optional(std::move(value))));
			blocker.reset();
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			assert(dispatcher);
			dispatcher->post(boost::bind<void>(action, boost::optional<Element>()));
			blocker.reset();
		}
	};

	template <class Element, class Action>
	auto blocking_then(boost::asio::io_service &io, Si::observable<Element> &from, Action &&action) -> std::shared_ptr<blocking_then_state<Element, typename std::decay<Action>::type>>
	{
		auto state = std::make_shared<blocking_then_state<Element, typename std::decay<Action>::type>>(io, std::forward<Action>(action));
		from.async_get_one(*state);
		state->from = &from;
		return state;
	}
}

namespace Si
{
	template <class Element>
	using signal_observer_map =
#ifdef _MSC_VER
		std::unordered_map
#else
		boost::container::flat_map
#endif
		<observer<Element> *, bool>;

	template <class Element>
	struct connection : observable<Element>
	{
		typedef Element element_type;

		connection()
		{
		}

		explicit connection(signal_observer_map<Element> &connections)
			: connections(&connections)
		{
		}

		connection(connection &&other)
			: connections(other.connections)
			, receiver_(other.receiver_)
		{
			other.connections = nullptr;
			other.receiver_ = nullptr;
		}

		connection &operator = (connection &&other)
		{
			boost::swap(connections, other.connections);
			boost::swap(receiver_, other.receiver_);
			return *this;
		}

		~connection()
		{
			if (!connections)
			{
				return;
			}
			connections->erase(receiver_);
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			auto * const old_receiver = receiver_;
			connections->insert(std::make_pair(&receiver, true)).first->second = true;
			if (old_receiver && (old_receiver != &receiver))
			{
				auto i = connections->find(receiver_);
				assert(i->second);
				connections->erase(i);
			}
			receiver_ = &receiver;
		}

	private:

		signal_observer_map<Element> *connections = nullptr;
		observer<Element> *receiver_ = nullptr;

		SILICIUM_DELETED_FUNCTION(connection(connection const &))
		SILICIUM_DELETED_FUNCTION(connection &operator = (connection const &))
	};

	template <class Element>
	struct signal : boost::noncopyable
	{
		typedef connection<Element> connection_type;

		connection_type connect()
		{
			return connection_type(observers);
		}

		void emit_one(Element const &value)
		{
			for (auto &observer : observers)
			{
				if (observer.second)
				{
					observer.second = false;
					observer.first->got_element(value);
				}
			}
		}

	private:

		signal_observer_map<Element> observers;
	};
}

namespace
{
	BOOST_AUTO_TEST_CASE(reactive_signal)
	{
		Si::signal<int> s;
		auto con1 = s.connect();
		auto con2 = s.connect();
		std::vector<int> generated;
		auto consumer = Si::consume<int>([&generated](boost::optional<int> value)
		{
			BOOST_REQUIRE(value);
			generated.emplace_back(*value);
		});
		con1.async_get_one(consumer);
		s.emit_one(2);
		con2 = std::move(con1);
		con2.async_get_one(consumer);
		s.emit_one(3);
		s.emit_one(4);
		std::vector<int> const expected{2, 3};
		BOOST_CHECK(expected == generated);
	}
}

BOOST_AUTO_TEST_CASE(reactive_timer)
{
	boost::asio::io_service io;
	Si::timer<> t(io, std::chrono::microseconds(1));
	std::size_t elapsed_count = 0;
	auto coro = Si::make_total_consumer(Si::make_coroutine<Si::nothing>([&t, &elapsed_count](Si::yield_context<> &yield)
	{
		BOOST_REQUIRE_EQUAL(0U, elapsed_count);
		boost::optional<Si::timer_elapsed> e = yield.get_one(t);
		BOOST_REQUIRE(e);
		++elapsed_count;
	}));
	coro.start();
	io.run();
	BOOST_CHECK_EQUAL(1U, elapsed_count);
}

BOOST_AUTO_TEST_CASE(reactive_async_empty)
{
	auto a = Si::make_thread<int, Si::std_threading>([](Si::yield_context<int> &)
	{
	});
	auto consumer = Si::consume<int>([](int element)
	{
		boost::ignore_unused_variable_warning(element);
		BOOST_FAIL("No element expected here");
	});
	a.async_get_one(consumer);
	a.wait();
}

BOOST_AUTO_TEST_CASE(reactive_async)
{
	auto a = Si::make_thread<int, Si::std_threading>([](Si::yield_context<int> &yield)
	{
		yield(1);
		yield(2);
		yield(3);
	});
	std::vector<int> const expected{1, 2, 3};
	std::vector<int> produced;
	std::unique_ptr<Si::observer<int>> pusher;
	pusher = Si::to_unique(Si::consume<int>([&](int element)
	{
		produced.emplace_back(element);
		if (produced.size() == expected.size())
		{
			return;
		}
		a.async_get_one(*pusher);
	}));
	a.async_get_one(*pusher);
	a.wait();
	BOOST_CHECK(expected == produced);
}

BOOST_AUTO_TEST_CASE(reactive_async_get_one)
{
	auto a = Si::make_thread<int, Si::std_threading>([](Si::yield_context<int> &yield)
	{
		auto b = Si::make_thread<int, Si::boost_threading>([](Si::yield_context<int> &yield)
		{
			yield(1);
			yield(2);
			yield(3);
		});
		for (;;)
		{
			auto e = yield.get_one(b);
			if (!e)
			{
				break;
			}
			yield(std::move(*e));
		}
		b.wait();
	});
	std::vector<int> const expected{1, 2, 3};
	std::vector<int> produced;
	std::unique_ptr<Si::observer<int>> pusher;
	pusher = Si::to_unique(Si::consume<int>([&](int element)
	{
		produced.emplace_back(element);
		if (produced.size() == expected.size())
		{
			return;
		}
		a.async_get_one(*pusher);
	}));
	a.async_get_one(*pusher);
	a.wait();
	BOOST_CHECK(expected == produced);
}
