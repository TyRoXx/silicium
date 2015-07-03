#ifndef SILICIUM_BOOST_THREADING_HPP
#define SILICIUM_BOOST_THREADING_HPP

#include <silicium/config.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/future.hpp>

namespace Si
{
	struct boost_threading
	{
#if SILICIUM_HAS_EXCEPTIONS
		template <class T>
		struct future
		{
			typedef boost::unique_future<T> type;
		};
		template <class T>
		struct promise
		{
			typedef boost::promise<T> type;
		};
		template <class Result>
		struct packaged_task
		{
			typedef boost::packaged_task<Result
#ifdef BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
				()
#endif
			> type;
		};
#endif
		typedef boost::mutex mutex;
		typedef boost::condition_variable condition_variable;
		typedef boost::unique_lock<boost::mutex> unique_lock;
#if (BOOST_VERSION >= 105000) && SILICIUM_HAS_EXCEPTIONS
		template <class Action, class ...Args>
		static auto launch_async(Action &&action, Args &&...args) -> boost::unique_future<decltype(action(std::forward<Args>(args)...))>
		{
			return boost::async(boost::launch::async, std::forward<Action>(action), std::forward<Args>(args)...);
		}
#endif
	};
}

#endif
