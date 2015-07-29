#include <silicium/buffer.hpp>
#include <silicium/observable/variant.hpp>
#include <silicium/observable/coroutine_generator.hpp>
#include <silicium/observable/generator.hpp>
#include <silicium/observable/consume.hpp>
#include <silicium/asio/timer.hpp>
#include <silicium/observable/tuple.hpp>
#include <silicium/observable/total_consumer.hpp>
#include <silicium/observable/ptr.hpp>
#include <silicium/observable/transform.hpp>
#include <silicium/observable/bridge.hpp>
#include <silicium/observable/enumerate.hpp>
#include <silicium/observable/constant.hpp>
#include <silicium/observable/cache.hpp>
#include <silicium/observable/empty.hpp>
#include <silicium/observable/thread_generator.hpp>
#include <silicium/boost_threading.hpp>
#include <silicium/std_threading.hpp>
#include <silicium/observable/deref_optional.hpp>
#include <silicium/to_unique.hpp>
#include <boost/container/string.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/thread.hpp>
#include <boost/mpl/list.hpp>
#include <unordered_map>
#if SILICIUM_HAS_EXCEPTIONS
#	include <future>
#endif

namespace Si
{
#if SILICIUM_RX_TUPLE_AVAILABLE
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
		Si::ptr_observable<int, std::shared_ptr<Si::observable<int, Si::ptr_observer<Si::observer<int>>>>> first(bridge);
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

		added.async_get_one(Si::observe_by_ref(consumer));
		BOOST_CHECK(generated.empty());

		bridge->got_element(2);
		std::vector<int> const expected(1, 3);
		BOOST_CHECK(expected == generated);
	}
#endif

	BOOST_AUTO_TEST_CASE(reactive_make_buffer)
	{
		auto bridge = std::make_shared<Si::bridge<int>>();
		Si::ptr_observable<int, std::shared_ptr<Si::Observable<int, Si::ptr_observer<Si::observer<int>>>::interface>> first{bridge};
		auto buf = Si::make_buffer_observable(first, 2);
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

		buf.async_get_one(Si::observe_by_ref(consumer));
		std::vector<int> expected(1, 7);
		BOOST_CHECK(expected == generated);

		buf.async_get_one(Si::observe_by_ref(consumer));
		expected.emplace_back(7);
		BOOST_CHECK(expected == generated);

		buf.async_get_one(Si::observe_by_ref(consumer));
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

		typedef Si::variant<int, string> variant;
		std::vector<variant> produced;
		auto consumer = Si::consume<variant>([&produced](variant element)
		{
			produced.emplace_back(std::move(element));
		});

		variants.async_get_one(Si::observe_by_ref(consumer));
		BOOST_CHECK(produced.empty());
		first.got_element(4);

		variants.async_get_one(Si::observe_by_ref(consumer));
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
		typename Si::Observable<Element, Si::ptr_observer<Si::observer<Element>>>::interface *from;

		explicit blocking_then_state(boost::asio::io_service &dispatcher, Action action)
			: dispatcher(&dispatcher)
			, blocker(boost::in_place(boost::ref(dispatcher)))
			, action(std::move(action))
			, from(nullptr)
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
	auto blocking_then(boost::asio::io_service &io, Si::observable<Element, Si::ptr_observer<Si::observer<Element>>> &from, Action &&action)
		-> std::shared_ptr<blocking_then_state<Element, typename std::decay<Action>::type>>
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
	struct signal_observer_map
	{
		typedef
#ifdef _MSC_VER
			std::unordered_map
#else
			boost::container::flat_map
#endif
			<observer<Element> *, bool> type;
	};

	template <class Element>
	struct connection : Observable<Element, ptr_observer<observer<Element>>>::interface
	{
		typedef Element element_type;

		connection()
			: connections(nullptr)
			, receiver_(nullptr)
		{
		}

		explicit connection(typename signal_observer_map<Element>::type &connections)
			: connections(&connections)
			, receiver_(nullptr)
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

		virtual void async_get_one(ptr_observer<observer<element_type>> receiver) SILICIUM_OVERRIDE
		{
			auto * const old_receiver = receiver_;
			connections->insert(std::make_pair(receiver.get(), true)).first->second = true;
			if (old_receiver && (old_receiver != receiver.get()))
			{
				auto i = connections->find(receiver_);
				assert(i->second);
				connections->erase(i);
			}
			receiver_ = receiver.get();
		}

	private:

		typename signal_observer_map<Element>::type *connections;
		observer<Element> *receiver_;

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

		typename signal_observer_map<Element>::type observers;
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
		con1.async_get_one(Si::observe_by_ref(consumer));
		s.emit_one(2);
		con2 = std::move(con1);
		con2.async_get_one(Si::observe_by_ref(consumer));
		s.emit_one(3);
		s.emit_one(4);
		std::vector<int> const expected{2, 3};
		BOOST_CHECK(expected == generated);
	}
}
