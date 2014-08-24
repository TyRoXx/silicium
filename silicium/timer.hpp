#ifndef SILICIUM_REACTIVE_TIMER_HPP
#define SILICIUM_REACTIVE_TIMER_HPP

#include <silicium/observer.hpp>
#include <silicium/exchange.hpp>
#include <silicium/override.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/chrono/duration.hpp>

namespace Si
{
	struct timer_elapsed
	{
	};

	template <class AsioTimer = boost::asio::steady_timer>
	struct timer
	{
		typedef timer_elapsed element_type;
		typedef AsioTimer timer_impl;
		typedef typename timer_impl::duration duration;

		template <class Duration>
		explicit timer(boost::asio::io_service &io, Duration delay)
			: impl(io)
			, delay(std::chrono::duration_cast<duration>(delay))
		{
		}

		void async_get_one(observer<element_type> &receiver)
		{
			assert(!receiver_);
			receiver_ = &receiver;
			impl.expires_from_now(delay);
			impl.async_wait([this](boost::system::error_code error)
			{
				if (error)
				{
					assert(error == boost::asio::error::operation_aborted); //TODO: remove this assumption
					assert(!this->receiver_); //cancel() should have reset the receiver already
					return;
				}
				Si::exchange(this->receiver_, nullptr)->got_element(timer_elapsed{});
			});
		}

		void cancel()
		{
			assert(receiver_);
			impl.cancel();
			receiver_ = nullptr;
		}

	private:

		timer_impl impl;
		duration delay;
		observer<element_type> *receiver_ = nullptr;
	};
}

#endif
