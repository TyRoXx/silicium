#ifndef SILICIUM_STD_THREADING_HPP
#define SILICIUM_STD_THREADING_HPP

#include <future>

namespace Si
{
	struct std_threading
	{
		template <class T>
		struct future
		{
			typedef std::future<T> type;
		};
		template <class T>
		struct promise
		{
			typedef std::promise<T> type;
		};
		template <class Result>
		struct packaged_task
		{
			typedef std::packaged_task<Result ()> type;
		};
		typedef std::mutex mutex;
		typedef std::condition_variable condition_variable;
		typedef std::unique_lock<std::mutex> unique_lock;
		template <class Action, class ...Args>
		static auto launch_async(Action &&action, Args &&...args) -> std::future<decltype(action(std::forward<Args>(args)...))>
		{
			return std::async(std::launch::async, std::forward<Action>(action), std::forward<Args>(args)...);
		}
	};
}

#endif
