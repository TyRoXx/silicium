#ifndef SILICIUM_ASIO_TIMER_HPP
#define SILICIUM_ASIO_TIMER_HPP

#include <silicium/observable/observer.hpp>
#include <silicium/exchange.hpp>
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

		template <class AsioTimer>
		struct timer
		{
			typedef timer_elapsed element_type;
			typedef AsioTimer timer_impl;
			typedef typename timer_impl::duration duration;

			explicit timer(boost::asio::io_service &io)
				: impl(make_unique<timer_impl>(io))
			{
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			timer(timer &&other) BOOST_NOEXCEPT = default;
			timer &operator = (timer &&other) BOOST_NOEXCEPT = default;
#else
			timer(timer &&other) BOOST_NOEXCEPT
				: impl(std::move(other.impl))
			{
			}

			timer &operator = (timer &&other) BOOST_NOEXCEPT
			{
				impl = std::move(other.impl);
				return *this;
			}
#endif

			template <class Duration>
			void expires_from_now(Duration &&delay)
			{
				assert(impl);
				impl->expires_from_now(delay);
			}

			template <class Observer>
			void async_get_one(Observer &&receiver)
			{
				assert(impl);
				impl->async_wait([this, receiver](boost::system::error_code error) mutable
				{
					if (error)
					{
						assert(error == boost::asio::error::operation_aborted); //TODO: remove this assumption
						return;
					}
					std::forward<Observer>(receiver).got_element(timer_elapsed{});
				});
			}

		private:

			std::unique_ptr<timer_impl> impl;

			SILICIUM_DELETED_FUNCTION(timer(timer const &))
			SILICIUM_DELETED_FUNCTION(timer &operator = (timer const &))
		};

		template <class AsioTimer = boost::asio::steady_timer>
		auto make_timer(boost::asio::io_service &io)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			-> timer<AsioTimer>
#endif
		{
			return timer<AsioTimer>(io);
		}
	}
}

#endif
