#include <silicium/coroutine.hpp>
#include <silicium/consume.hpp>
#include <boost/test/unit_test.hpp>

namespace Si
{
	namespace detail
	{
		struct delivered
		{
		};

		template <class Message>
		struct channel_common_state
		{
			observer<Message> *receiver = nullptr;
			observer<delivered> *delivery = nullptr;
			Message *message = nullptr;
		};

		template <class Message>
		struct channel_receiving_end
		{
			using element_type = Message;

			explicit channel_receiving_end(channel_common_state<Message> &state)
				: state(&state)
			{
			}

			void async_get_one(observer<element_type> &receiver)
			{
				assert(!state->receiver);
				if (state->message)
				{
					receiver.got_element(std::move(*Si::exchange(state->message, nullptr)));
					Si::exchange(state->delivery, nullptr)->got_element(delivered());
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

			channel_common_state<Message> *state;
		};

		template <class Message>
		struct channel_sending_end
		{
			using element_type = delivered;

			explicit channel_sending_end(channel_common_state<Message> &state)
				: state(&state)
			{
			}

			void set_message(Message &message)
			{
				assert(!state->message);
				if (state->receiver)
				{
					Si::exchange(state->receiver, nullptr)->got_element(std::move(message));
				}
				else
				{
					state->message = &message;
				}
			}

			void async_get_one(observer<element_type> &receiver)
			{
				assert(!state->delivery);
				if (state->message)
				{
					state->delivery = &receiver;
				}
				else
				{
					assert(!state->receiver);
					receiver.got_element(delivered());
				}
			}

			void cancel()
			{
			}

		private:

			channel_common_state<Message> *state;
		};
	}

	template <class Message>
	struct channel
	{
		channel()
			: receiving(state)
			, sending(state)
		{
		}

		detail::channel_receiving_end<Message> &receiver()
		{
			return receiving;
		}

		detail::channel_sending_end<Message> &sender()
		{
			return sending;
		}

	private:

		detail::channel_common_state<Message> state;
		detail::channel_receiving_end<Message> receiving;
		detail::channel_sending_end<Message> sending;
	};

	template <class Message, class YieldContext>
	void send(channel<Message> &chan, Message message, YieldContext &yield)
	{
		chan.sender().set_message(message);
		yield.get_one(chan.sender());
	}

	template <class Message, class YieldContext>
	boost::optional<Message> receive(channel<Message> &chan, YieldContext &yield)
	{
		return yield.get_one(chan.receiver());
	}
}

BOOST_AUTO_TEST_CASE(channel_)
{
	Si::channel<int> channel;
	auto t = Si::make_coroutine<int>([&channel](Si::yield_context<int> &yield)
	{
		Si::send(channel, 2, yield);
		Si::send(channel, 3, yield);
	});
	auto s = Si::make_coroutine<int>([&channel](Si::yield_context<int> &yield)
	{
		auto a = Si::receive(channel, yield);
		BOOST_REQUIRE(a);
		auto b = Si::receive(channel, yield);
		BOOST_REQUIRE(b);
		int result = *a + *b;
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
