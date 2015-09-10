#ifndef SILICIUM_FUTURE_HPP
#define SILICIUM_FUTURE_HPP

#include <silicium/variant.hpp>
#include <boost/asio/async_result.hpp>

#define SILICIUM_HAS_FUTURE SILICIUM_HAS_VARIANT

#if SILICIUM_HAS_FUTURE
namespace Si
{
	template <class T>
	struct future
	{
		future()
		{
		}

		explicit future(T value)
			: m_state(std::move(value))
		{
		}

		template <class Handler>
		auto async_wait(Handler &&handler)
			-> typename boost::asio::async_result<typename boost::asio::handler_type<Handler, void(T)>::type>::type
		{
			typename boost::asio::handler_type<Handler, void(T)>::type real_handler(std::forward<Handler>(handler));
			typename boost::asio::async_result<decltype(real_handler)> result(real_handler);
			visit<void>(
				m_state,
				[](empty)
				{
					throw std::logic_error("to do");
				},
				[this, &real_handler](T &value)
				{
					real_handler(std::forward<T>(value));
					m_state = empty();
				}
			);
			return result.get();
		}

	private:

		struct empty
		{
		};

		variant<empty, T> m_state;
	};

	template <class T>
	future<typename std::decay<T>::type> make_ready_future(T &&value)
	{
		return future<typename std::decay<T>::type>(std::forward<T>(value));
	}

	template <class T>
	struct promise
	{

	};
}
#endif

#endif
