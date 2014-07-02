#ifndef SILICIUM_REACTIVE_TIMER_HPP
#define SILICIUM_REACTIVE_TIMER_HPP

#include <reactive/observable.hpp>
#include <reactive/exchange.hpp>
#include <boost/asio/deadline_timer.hpp>

namespace rx
{
	struct timer_elapsed
	{
	};

	struct timer : observable<timer_elapsed>
	{
		typedef timer_elapsed element_type;

		explicit timer(boost::asio::io_service &io, boost::posix_time::time_duration delay)
			: impl(io)
			, delay(delay)
		{
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(!receiver_);
			receiver_ = &receiver;
			impl.expires_from_now(delay);
			impl.async_wait([this](boost::system::error_code error)
			{
				if (error)
				{
					assert(error == boost::asio::error::operation_aborted); //TODO: remove this assumption
					assert(!receiver_); //cancel() should have reset the receiver already
					return;
				}
				exchange(receiver_, nullptr)->got_element(timer_elapsed{});
			});
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			assert(receiver_);
			impl.cancel();
			receiver_ = nullptr;
		}

	private:

		boost::asio::deadline_timer impl;
		boost::posix_time::time_duration delay;
		observer<element_type> *receiver_ = nullptr;
	};
}

#endif
