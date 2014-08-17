#ifndef SILICIUM_THREAD_HPP
#define SILICIUM_THREAD_HPP

#include <silicium/observer.hpp>
#include <silicium/override.hpp>
#include <future>
#include <boost/thread/future.hpp>

namespace Si
{
	template <class Element>
	struct yield_context_2
	{
		virtual ~yield_context_2()
		{
		}

		virtual void push_result(Element result) = 0;
	};

	template <class Element, class ThreadingAPI>
	struct thread_observable : public observable<Element>
	{
		typedef Element element_type;

		thread_observable()
		{
		}

		template <class Action>
		explicit thread_observable(Action &&action)
			: state(make_unique<movable_state>(std::forward<Action>(action)))
		{
		}

		void wait()
		{
			assert(state);
			state->thread.get();
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(state);
			typename ThreadingAPI::unique_lock lock(state->result_mutex);
			assert(!state->receiver_);
			state->receiver_ = &receiver;
			if (!state->cached_result)
			{
				if (!state->thread.valid())
				{
					lock.unlock();
					return Si::exchange(state->receiver_, nullptr)->ended();
				}
				return;
			}
			auto ready_result = std::move(*state->cached_result);
			state->result_retrieved.notify_one();
			lock.unlock();
			Si::exchange(state->receiver_, nullptr)->got_element(std::move(ready_result));
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			throw std::logic_error("to do");
		}

	private:

		struct movable_state : private yield_context_2<Element>
		{
			typename ThreadingAPI::template future<void> thread;
			typename ThreadingAPI::mutex result_mutex;
			typename ThreadingAPI::condition_variable result_retrieved;
			observer<element_type> *receiver_ = nullptr;
			boost::optional<Element> cached_result;

			template <class Action>
			explicit movable_state(Action &&action)
			{
				//start the background thread after the initialization of all the members
				thread = ThreadingAPI::template launch_async([this, action]() -> void
				{
					action(static_cast<yield_context_2<Element> &>(*this));
				});
			}

			virtual void push_result(Element result) SILICIUM_OVERRIDE
			{
				typename ThreadingAPI::unique_lock lock(result_mutex);
				while (cached_result.is_initialized())
				{
					result_retrieved.wait(lock);
				}
				if (receiver_)
				{
					auto receiver = Si::exchange(receiver_, nullptr);
					lock.unlock();
					receiver->got_element(std::move(result));
				}
				else
				{
					cached_result = std::move(result);
				}
			}
		};

		std::unique_ptr<movable_state> state;
	};

	struct std_threading
	{
		template <class T>
		using future = std::future<T>;
		using mutex = std::mutex;
		using condition_variable = std::condition_variable;
		using unique_lock = std::unique_lock<std::mutex>;
		template <class Action, class ...Args>
		static auto launch_async(Action &&action, Args &&...args) -> std::future<decltype(action(std::forward<Args>(args)...))>
		{
			return std::async(std::launch::async, std::forward<Action>(action), std::forward<Args>(args)...);
		}
	};

	struct boost_threading
	{
		template <class T>
		using future = boost::unique_future<T>;
		using mutex = boost::mutex;
		using condition_variable = boost::condition_variable;
		using unique_lock = boost::unique_lock<boost::mutex>;
		template <class Action, class ...Args>
		static auto launch_async(Action &&action, Args &&...args) -> boost::unique_future<decltype(action(std::forward<Args>(args)...))>
		{
			return boost::async(boost::launch::async, std::forward<Action>(action), std::forward<Args>(args)...);
		}
	};

	template <class Element, class ThreadingAPI, class Action>
	auto make_thread(Action &&action) -> thread_observable<Element, ThreadingAPI>
	{
		return thread_observable<Element, ThreadingAPI>(std::forward<Action>(action));
	}
}

#endif
