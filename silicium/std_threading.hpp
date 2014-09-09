#ifndef SILICIUM_STD_THREADING_HPP
#define SILICIUM_STD_THREADING_HPP

#include <future>

namespace Si
{
	struct std_threading
	{
		template <class T>
		using future = std::future<T>;
		template <class T>
		using promise = std::promise<T>;
		using mutex = std::mutex;
		using condition_variable = std::condition_variable;
		using unique_lock = std::unique_lock<std::mutex>;
		template <class Action, class ...Args>
		static auto launch_async(Action &&action, Args &&...args) -> std::future<decltype(action(std::forward<Args>(args)...))>
		{
			return std::async(std::launch::async, std::forward<Action>(action), std::forward<Args>(args)...);
		}
	};
}

#endif
