#ifndef SILICIUM_BOOST_THREADING_HPP
#define SILICIUM_BOOST_THREADING_HPP

#include <boost/thread/future.hpp>

namespace Si
{
	struct boost_threading
	{
		template <class T>
		using future = boost::unique_future<T>;
		template <class T>
		using promise = boost::promise<T>;
		using mutex = boost::mutex;
		using condition_variable = boost::condition_variable;
		using unique_lock = boost::unique_lock<boost::mutex>;
		template <class Action, class ...Args>
		static auto launch_async(Action &&action, Args &&...args) -> boost::unique_future<decltype(action(std::forward<Args>(args)...))>
		{
			return boost::async(boost::launch::async, std::forward<Action>(action), std::forward<Args>(args)...);
		}
	};
}

#endif
