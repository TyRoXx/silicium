#include <silicium/coroutine.hpp>
#include <silicium/consume.hpp>
#include <silicium/thread.hpp>
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
			observer<Message> *receiver = nullptr;
			observer<delivered> *delivery = nullptr;
			Message *message = nullptr;
		};

		template <class Message, class Lockable>
		struct channel_receiving_end
		{
			using element_type = Message;

			explicit channel_receiving_end(channel_common_state<Message, Lockable> &state)
				: state(&state)
			{
			}

			void async_get_one(observer<element_type> &receiver)
			{
				boost::unique_lock<boost::mutex> lock(state->access);
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

			void cancel()
			{
			}

		private:

			channel_common_state<Message, Lockable> *state;
		};

		template <class Message, class Lockable>
		struct channel_sending_end
		{
			using element_type = delivered;

			explicit channel_sending_end(channel_common_state<Message, Lockable> &state)
				: state(&state)
			{
			}

			void set_message(Message &message)
			{
				//TODO: combine this lock with the one in async_get_one
				boost::unique_lock<boost::mutex> lock(state->access);
				assert(!state->message);
				if (state->receiver)
				{
					auto * const receiver = Si::exchange(state->receiver, nullptr);
					lock.unlock();
					receiver->got_element(std::move(message));
				}
				else
				{
					state->message = &message;
				}
			}

			void async_get_one(observer<element_type> &receiver)
			{
				boost::unique_lock<boost::mutex> lock(state->access);
				assert(!state->delivery);
				if (state->message)
				{
					state->delivery = &receiver;
				}
				else
				{
					assert(!state->receiver);
					lock.unlock();
					receiver.got_element(delivered());
				}
			}

			void cancel()
			{
			}

		private:

			channel_common_state<Message, Lockable> *state;
		};
	}

	template <class Message, class ThreadingAPI = boost_threading>
	struct channel
	{
		//TODO: use a simpler spinlock
		using mutex = typename ThreadingAPI::mutex;

		channel()
			: receiving(state)
			, sending(state)
		{
		}

		template <class YieldContext>
		void send(Message message, YieldContext &yield)
		{
			sending.set_message(message);
			yield.get_one(sending);
		}

		template <class YieldContext>
		boost::optional<Message> receive(YieldContext &yield)
		{
			return yield.get_one(receiving);
		}

	private:

		detail::channel_common_state<Message, mutex> state;
		detail::channel_receiving_end<Message, mutex> receiving;
		detail::channel_sending_end<Message, mutex> sending;
	};
}

BOOST_AUTO_TEST_CASE(channel_with_coroutine)
{
	Si::channel<int> channel;
	auto t = Si::make_coroutine<int>([&channel](Si::yield_context<int> &yield)
	{
		channel.send(2, yield);
		channel.send(3, yield);
	});
	auto s = Si::make_coroutine<int>([&channel](Si::yield_context<int> &yield)
	{
		auto a = channel.receive(yield);
		BOOST_REQUIRE(a);
		BOOST_CHECK_EQUAL(2, *a);
		auto b = channel.receive(yield);
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
	auto t = Si::make_thread<int, Si::boost_threading>([&channel](Si::yield_context<int> &yield)
	{
		channel.send(2, yield);
		channel.send(3, yield);
	});
	auto s = Si::make_thread<int, Si::boost_threading>([&channel](Si::yield_context<int> &yield)
	{
		auto a = channel.receive(yield);
		BOOST_REQUIRE(a);
		BOOST_CHECK_EQUAL(2, *a);
		auto b = channel.receive(yield);
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
