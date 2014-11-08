#ifndef SILICIUM_REACTIVE_TIMER_HPP
#define SILICIUM_REACTIVE_TIMER_HPP

#include <silicium/observable/observer.hpp>
#include <silicium/exchange.hpp>
#include <silicium/override.hpp>
#include <silicium/config.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/chrono/duration.hpp>

namespace Si
{
	namespace asio
	{
		struct timer_elapsed
		{
		};

		template <class AsioTimer, class DurationObservable>
		struct timer : private observer<typename DurationObservable::element_type>
		{
			typedef timer_elapsed element_type;
			typedef AsioTimer timer_impl;
			typedef typename timer_impl::duration duration;

			explicit timer(boost::asio::io_service &io, DurationObservable delays)
				: impl(make_unique<timer_impl>(io))
				, delays(std::move(delays))
				, receiver_(nullptr)
			{
			}

			void async_get_one(observer<element_type> &receiver)
			{
				assert(!receiver_);
				receiver_ = &receiver;
				return delays.async_get_one(static_cast<observer<typename DurationObservable::element_type> &>(*this));
			}

		private:

			std::unique_ptr<timer_impl> impl;
			DurationObservable delays;
			observer<element_type> *receiver_;

			virtual void got_element(typename DurationObservable::element_type delay) SILICIUM_OVERRIDE
			{
				assert(receiver_);
				impl->expires_from_now(delay);
				impl->async_wait([this](boost::system::error_code error)
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

			virtual void ended() SILICIUM_OVERRIDE
			{
				return Si::exchange(receiver_, nullptr)->ended();
			}
		};

		template <class AsioTimer = boost::asio::steady_timer, class DurationObservable>
		auto make_timer(boost::asio::io_service &io, DurationObservable &&delays)
		{
			return timer<AsioTimer, typename std::decay<DurationObservable>::type>(io, std::forward<DurationObservable>(delays));
		}
	}
}

#endif
