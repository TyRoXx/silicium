#include <silicium/observable/coroutine_generator.hpp>
#include <silicium/observable/consume.hpp>
#include <silicium/observable/thread.hpp>
#include <silicium/observable/function.hpp>
#include <silicium/observable/variant.hpp>
#include <silicium/std_threading.hpp>
#include <silicium/boost_threading.hpp>
#include <boost/test/unit_test.hpp>

namespace Si
{
	namespace detail
	{
		struct delivered
		{
		};

		template <class Message, class Lockable>
		struct channel_common_state
		{
			Lockable access;
			observer<Message> *receiver;
			observer<delivered> *delivery;
			Message *message;

			channel_common_state()
				: receiver(nullptr)
				, delivery(nullptr)
				, message(nullptr)
			{
			}
		};

		template <class Message, class Lockable>
		struct channel_receiving_end
		{
			typedef Message element_type;

			explicit channel_receiving_end(channel_common_state<Message, Lockable> &state)
				: state(&state)
			{
			}

			void async_get_one(observer<element_type> &receiver)
			{
				boost::unique_lock<Lockable> lock(state->access);
				assert(!state->receiver);
				if (state->message)
				{
					auto * const message = Si::exchange(state->message, nullptr);
					auto * const delivery = Si::exchange(state->delivery, nullptr);
					lock.unlock();
					receiver.got_element(std::move(*message));
					delivery->got_element(delivered());
				}
				else
				{
					state->receiver = &receiver;
				}
			}

		private:

			channel_common_state<Message, Lockable> *state;
		};

		template <class Message, class Lockable>
		struct channel_sending_end
		{
			typedef delivered element_type;

			explicit channel_sending_end(channel_common_state<Message, Lockable> &state)
				: state(&state)
			{
			}
			
			void async_get_one(observer<element_type> &receiver, Message &message)
			{
				boost::unique_lock<Lockable> lock(state->access);
				assert(!state->message);
				assert(!state->delivery);
				if (state->receiver)
				{
					auto * const message_receiver = Si::exchange(state->receiver, nullptr);
					lock.unlock();
					message_receiver->got_element(std::move(message));
					receiver.got_element(delivered());
				}
				else
				{
					state->message = &message;
					state->delivery = &receiver;
				}
			}

		private:

			channel_common_state<Message, Lockable> *state;
		};
	}

	template <class Message, class ThreadingAPI = std_threading>
	struct channel
	{
		//TODO: use a simpler spinlock
		typedef typename ThreadingAPI::mutex mutex;

		channel()
			: receiving(detail::channel_receiving_end<Message, mutex>(state))
			, sending(state)
		{
		}

		template <class YieldContext>
		void send(Message message, YieldContext &yield)
		{
			auto message_bound = make_function_observable<detail::delivered>([this, &message](observer<detail::delivered> &receiver)
			{
				return sending.async_get_one(receiver, message);
			});
			yield.get_one(message_bound);
		}

		observable<Message> &receiver()
		{
			return receiving;
		}

	private:

		detail::channel_common_state<Message, mutex> state;
		virtualized_observable<detail::channel_receiving_end<Message, mutex>> receiving;
		detail::channel_sending_end<Message, mutex> sending;
	};
}

BOOST_AUTO_TEST_CASE(channel_with_coroutine)
{
	Si::channel<int> channel;
	auto t = Si::make_coroutine_generator<int>([&channel](Si::push_context<int> &yield)
	{
		channel.send(2, yield);
		channel.send(3, yield);
	});
	auto s = Si::make_coroutine_generator<int>([&channel](Si::push_context<int> &yield)
	{
		auto a = yield.get_one(channel.receiver());
		BOOST_REQUIRE(a);
		BOOST_CHECK_EQUAL(2, *a);
		auto b = yield.get_one(channel.receiver());
		BOOST_REQUIRE(b);
		BOOST_CHECK_EQUAL(3, *b);
		int result = *a + *b;
		BOOST_CHECK_EQUAL(5, result);
		yield(result);
	});
	bool got = false;
	auto consumer = Si::consume<int>([&s, &got](int result)
	{
		BOOST_CHECK(!got);
		got = true;
		BOOST_CHECK_EQUAL(5, result);
	});
	t.async_get_one(consumer);
	s.async_get_one(consumer);
	BOOST_CHECK(got);
}

BOOST_AUTO_TEST_CASE(channel_with_thread)
{
	Si::channel<int> channel;
	auto t = Si::make_thread<int, Si::boost_threading>([&channel](Si::push_context<int> &yield)
	{
		channel.send(2, yield);
		channel.send(3, yield);
	});
	auto s = Si::make_thread<int, Si::boost_threading>([&channel](Si::push_context<int> &yield)
	{
		auto a = yield.get_one(channel.receiver());
		BOOST_REQUIRE(a);
		BOOST_CHECK_EQUAL(2, *a);
		auto b = yield.get_one(channel.receiver());
		BOOST_REQUIRE(b);
		BOOST_CHECK_EQUAL(3, *b);
		int result = *a + *b;
		BOOST_CHECK_EQUAL(5, result);
		yield(result);
	});
	bool got = false;
	auto consumer = Si::consume<int>([&s, &got](int result)
	{
		BOOST_CHECK(!got);
		got = true;
		BOOST_CHECK_EQUAL(5, result);
	});
	t.async_get_one(consumer);
	s.async_get_one(consumer);
	t.wait();
	s.wait();
	BOOST_CHECK(got);
}

#if SILICIUM_RX_VARIANT_AVAILABLE
BOOST_AUTO_TEST_CASE(channel_select)
{
	Si::channel<int> channel_1;
	Si::channel<long> channel_2;
	auto t = Si::make_thread<long, Si::boost_threading>([&](Si::push_context<long> &yield)
	{
		channel_1.send(2, yield);
		channel_2.send(3L, yield);
	});
	auto s = Si::make_thread<long, Si::boost_threading>([&](Si::push_context<long> &yield)
	{
		auto both = Si::make_variant(Si::ref(channel_1.receiver()), Si::ref(channel_2.receiver()));
		boost::optional<Si::fast_variant<int, long>> a = yield.get_one(both);
		BOOST_REQUIRE(a);
		boost::optional<Si::fast_variant<int, long>> b = yield.get_one(both);
		BOOST_REQUIRE(b);
		long const result = Si::visit<long>(
			*a,
			[&b](int left)
		{
			return Si::visit<long>(
				*b,
				[](int) -> long { BOOST_FAIL("wrong type"); SILICIUM_UNREACHABLE(); },
				[left](long right) { return left + right; });
		},
			[](long) -> long { BOOST_FAIL("wrong type"); SILICIUM_UNREACHABLE(); });
		BOOST_CHECK_EQUAL(5L, result);
		yield(result);
	});
	bool got = false;
	auto consumer = Si::consume<long>([&s, &got](long result)
	{
		BOOST_CHECK(!got);
		got = true;
		BOOST_CHECK_EQUAL(5, result);
	});
	t.async_get_one(consumer);
	s.async_get_one(consumer);
	t.wait();
	s.wait();
	BOOST_CHECK(got);
}
#endif
