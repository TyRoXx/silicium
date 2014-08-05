#include <reactive/buffer.hpp>
#include <reactive/process.hpp>
#include <reactive/variant.hpp>
#include <reactive/coroutine.hpp>
#include <reactive/generate.hpp>
#include <reactive/consume.hpp>
#include <reactive/timer.hpp>
#include <reactive/tuple.hpp>
#include <reactive/total_consumer.hpp>
#include <reactive/ptr_observable.hpp>
#include <reactive/transform.hpp>
#include <reactive/bridge.hpp>
#include <reactive/take.hpp>
#include <reactive/enumerate.hpp>
#include <reactive/cache.hpp>
#include <reactive/connector.hpp>
#include <reactive/deref_optional.hpp>
#include <boost/container/string.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/thread.hpp>
#include <unordered_map>
#include <future>

namespace rx
{
	template <class Element>
	struct empty : observable<Element>
	{
		virtual void async_get_one(observer<Element> &receiver) SILICIUM_OVERRIDE
		{
			return receiver.ended();
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			throw std::logic_error("empty observable cannot be cancelled");
		}
	};
}

namespace Si
{
#if SILICIUM_RX_TUPLE_AVAILABLE
	BOOST_AUTO_TEST_CASE(reactive_take)
	{
		auto zeros = rx::generate([]{ return 0; });
		auto ones  = rx::generate([]{ return 1; });
		auto both = rx::make_tuple(zeros, ones);
		std::vector<std::tuple<int, int>> const expected(4, std::make_tuple(0, 1));
		std::vector<std::tuple<int, int>> const generated = rx::take(both, expected.size());
		BOOST_CHECK(expected == generated);
	}
#endif

#if SILICIUM_RX_TUPLE_AVAILABLE
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
#endif

#if SILICIUM_RX_TUPLE_AVAILABLE
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
#endif

	BOOST_AUTO_TEST_CASE(reactive_make_buffer)
	{
		auto bridge = std::make_shared<rx::bridge<int>>();
		rx::ptr_observable<int, std::shared_ptr<rx::observable<int>>> first{bridge};
		auto buf = rx::make_buffer(first, 2);
		buf.prefetch();

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

	BOOST_AUTO_TEST_CASE(reactive_coroutine_generate)
	{
		auto co = rx::make_coroutine<int>([](rx::yield_context<int> &yield) -> void
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
		bool exited_cleanly = false;
		auto co = rx::make_coroutine<int>([&asyncs, &exited_cleanly](rx::yield_context<int> &yield) -> void
		{
			auto a = yield.get_one(asyncs);
			BOOST_REQUIRE(a);
			yield(*a - 1);
			exited_cleanly = true;
		});
		std::vector<int> generated;
		auto collector = rx::consume<int>([&generated](int element)
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
		rx::bridge<int> first;
		rx::bridge<boost::container::string> second;
		auto variants = make_variant(rx::ref(first), rx::ref(second));

		typedef Si::fast_variant<int, boost::container::string> variant;
		std::vector<variant> produced;
		auto consumer = rx::consume<variant>([&produced](variant element)
		{
			produced.emplace_back(std::move(element));
		});

		variants.async_get_one(consumer);
		BOOST_CHECK(produced.empty());
		first.got_element(4);

		variants.async_get_one(consumer);
		BOOST_CHECK_EQUAL(1,produced.size());
		second.got_element("Hi");

		std::vector<variant> const expected
		{
			4,
			boost::container::string("Hi")
		};

		BOOST_CHECK(expected == produced);
		BOOST_CHECK(!rx::get_immediate(variants));
	}
#endif

	template <class Element, class Action>
	struct blocking_then_state : rx::observer<Element>
	{
		boost::asio::io_service *dispatcher;
		boost::optional<boost::asio::io_service::work> blocker;
		Action action;
		rx::observable<Element> *from = nullptr;

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
	auto blocking_then(boost::asio::io_service &io, rx::observable<Element> &from, Action &&action) -> std::shared_ptr<blocking_then_state<Element, typename std::decay<Action>::type>>
	{
		auto state = std::make_shared<blocking_then_state<Element, typename std::decay<Action>::type>>(io, std::forward<Action>(action));
		from.async_get_one(*state);
		state->from = &from;
		return state;
	}

	BOOST_AUTO_TEST_CASE(reactive_process)
	{
		rx::empty<char> input;
		rx::process proc = rx::launch_process("/usr/bin/which", {"which"}, input);

		boost::asio::io_service io;
		auto blocking = blocking_then(io, proc.exit_code, [](boost::optional<int> ec)
		{
			BOOST_CHECK_EQUAL(0, ec);
		});
		io.run();
	}
}

namespace rx
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

		virtual void cancel() SILICIUM_OVERRIDE
		{
			assert(receiver_);
			size_t erased = connections->erase(exchange(receiver_, nullptr));
			assert(erased == 1);
		}

	private:

		signal_observer_map<Element> *connections = nullptr;
		observer<Element> *receiver_ = nullptr;

		BOOST_DELETED_FUNCTION(connection(connection const &))
		BOOST_DELETED_FUNCTION(connection &operator = (connection const &))
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
		rx::signal<int> s;
		auto con1 = s.connect();
		auto con2 = s.connect();
		std::vector<int> generated;
		auto consumer = rx::consume<int>([&generated](boost::optional<int> value)
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
	rx::timer<> t(io, std::chrono::microseconds(1));
	std::size_t elapsed_count = 0;
	auto coro = rx::make_total_consumer(rx::make_coroutine<rx::nothing>([&t, &elapsed_count](rx::yield_context<> &yield)
	{
		BOOST_REQUIRE_EQUAL(0, elapsed_count);
		boost::optional<rx::timer_elapsed> e = yield.get_one(t);
		BOOST_REQUIRE(e);
		++elapsed_count;
	}));
	coro.start();
	io.run();
	BOOST_CHECK_EQUAL(1, elapsed_count);
}

namespace rx
{
	template <class Element>
	struct yield_context_2
	{
		virtual ~yield_context_2()
		{
		}

		virtual void push_result(Element result) = 0;
	};

	template <class Element, class ThreadingAPI>
	struct async_observable : public observable<Element>
	{
		typedef Element element_type;

		async_observable()
		{
		}

		template <class Action>
		explicit async_observable(Action &&action)
			: state(make_unique<movable_state>(std::forward<Action>(action)))
		{
		}

		void wait()
		{
			assert(state);
			state->thread.get();
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(state);
			typename ThreadingAPI::unique_lock lock(state->result_mutex);
			assert(!state->receiver_);
			state->receiver_ = &receiver;
			if (!state->cached_result)
			{
				if (!state->thread.valid())
				{
					lock.unlock();
					return rx::exchange(state->receiver_, nullptr)->ended();
				}
				return;
			}
			auto ready_result = std::move(*state->cached_result);
			state->result_retrieved.notify_one();
			lock.unlock();
			rx::exchange(state->receiver_, nullptr)->got_element(std::move(ready_result));
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			throw std::logic_error("to do");
		}

	private:

		struct movable_state : private yield_context_2<Element>
		{
			typename ThreadingAPI::template future<void> thread;
			typename ThreadingAPI::mutex result_mutex;
			typename ThreadingAPI::condition_variable result_retrieved;
			observer<element_type> *receiver_ = nullptr;
			boost::optional<Element> cached_result;

			template <class Action>
			explicit movable_state(Action &&action)
			{
				//start the background thread after the initialization of all the members
				thread = ThreadingAPI::template launch_async([this, action]() -> void
				{
					action(static_cast<yield_context_2<Element> &>(*this));
				});
			}

			virtual void push_result(Element result) SILICIUM_OVERRIDE
			{
				typename ThreadingAPI::unique_lock lock(result_mutex);
				while (cached_result.is_initialized())
				{
					result_retrieved.wait(lock);
				}
				if (receiver_)
				{
					auto receiver = rx::exchange(receiver_, nullptr);
					lock.unlock();
					receiver->got_element(std::move(result));
				}
				else
				{
					cached_result = std::move(result);
				}
			}
		};

		std::unique_ptr<movable_state> state;
	};

	struct std_threading
	{
		template <class T>
		using future = std::future<T>;
		using mutex = std::mutex;
		using condition_variable = std::condition_variable;
		using unique_lock = std::unique_lock<std::mutex>;
		template <class Action, class ...Args>
		static auto launch_async(Action &&action, Args &&...args) -> std::future<decltype(action(std::forward<Args>(args)...))>
		{
			return std::async(std::launch::async, std::forward<Action>(action), std::forward<Args>(args)...);
		}
	};

	struct boost_threading
	{
		template <class T>
		using future = boost::unique_future<T>;
		using mutex = boost::mutex;
		using condition_variable = boost::condition_variable;
		using unique_lock = boost::unique_lock<boost::mutex>;
		template <class Action, class ...Args>
		static auto launch_async(Action &&action, Args &&...args) -> boost::unique_future<decltype(action(std::forward<Args>(args)...))>
		{
			return boost::async(boost::launch::async, std::forward<Action>(action), std::forward<Args>(args)...);
		}
	};

	template <class Element, class ThreadingAPI, class Action>
	auto async(Action &&action) -> async_observable<Element, ThreadingAPI>
	{
		return async_observable<Element, ThreadingAPI>(std::forward<Action>(action));
	}
}

typedef boost::mpl::list<rx::std_threading, rx::boost_threading> threading_apis;

BOOST_AUTO_TEST_CASE_TEMPLATE(reactive_async, ThreadingAPI, threading_apis)
{
	auto a = rx::async<int, rx::std_threading>([](rx::yield_context_2<int> &yield)
	{
		yield.push_result(1);
		yield.push_result(2);
		yield.push_result(3);
	});
	std::vector<int> const expected{1, 2, 3};
	std::vector<int> produced;
	std::unique_ptr<rx::observer<int>> pusher;
	pusher = rx::to_unique(rx::consume<int>([&](int element)
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
