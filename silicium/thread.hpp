#ifndef SILICIUM_THREAD_HPP
#define SILICIUM_THREAD_HPP

#include <silicium/observer.hpp>
#include <silicium/override.hpp>
#include <silicium/config.hpp>
#include <silicium/exchange.hpp>
#include <silicium/yield_context.hpp>
#include <silicium/threading.hpp>
#include <future>
#include <boost/thread/future.hpp>
#include <boost/optional.hpp>
#include <boost/concept_check.hpp>

namespace Si
{
	template <class Element, class ThreadingAPI>
	struct thread_observable
	{
		typedef Element element_type;

		thread_observable()
		{
		}

		template <class Action>
		explicit thread_observable(Action &&action)
			: state(Si::make_unique<state_type>(std::forward<Action>(action)))
		{
		}

		void wait()
		{
			return state->wait();
		}

		void async_get_one(observer<element_type> &receiver)
		{
			return state->async_get_one(receiver);
		}

		void cancel()
		{
			return state->cancel();
		}

	private:

		struct state_type
			: private detail::yield_context_impl<Element>
			, private observer<nothing>
		{
			template <class Action>
			explicit state_type(Action &&action)
			{
				worker = ThreadingAPI::launch_async([action, this]() mutable
				{
					yield_context<Element> yield(*this);
					(std::forward<Action>(action))(yield);

					typename ThreadingAPI::unique_lock lock(receiver_mutex);
					if (receiver)
					{
						Si::exchange(receiver, nullptr)->ended();
					}
					else
					{
						has_ended = true;
					}
				});
			}

			void async_get_one(observer<Element> &new_receiver)
			{
				typename ThreadingAPI::unique_lock lock(receiver_mutex);
				assert(!receiver);
				if (ready_result)
				{
					Element result = std::move(*ready_result);
					ready_result = boost::none;
					receiver_ready.notify_one();
					lock.unlock();
					new_receiver.got_element(std::move(result));
				}
				else
				{
					if (has_ended)
					{
						return new_receiver.ended();
					}
					receiver = &new_receiver;
					receiver_ready.notify_one();
				}
			}

			void cancel()
			{
				throw std::logic_error("to do");
			}

			void wait()
			{
				worker.get();
			}

		private:

			typename ThreadingAPI::template future<void> worker;
			typename ThreadingAPI::mutex receiver_mutex;
			observer<Element> *receiver = nullptr;
			boost::optional<Element> ready_result;
			typename ThreadingAPI::condition_variable receiver_ready;
			bool has_ended = false;

			typename ThreadingAPI::mutex got_something_mutex;
			typename ThreadingAPI::condition_variable got_something_set;
			bool got_something = false;

			virtual void push_result(Element result) SILICIUM_OVERRIDE
			{
				typename ThreadingAPI::unique_lock lock(receiver_mutex);
				while (ready_result && !receiver)
				{
					receiver_ready.wait(lock);
				}
				if (receiver)
				{
					auto * const receiver_ = Si::exchange(receiver, nullptr);
					lock.unlock();
					receiver_->got_element(std::move(result));
				}
				else
				{
					ready_result = std::move(result);
				}
			}

			virtual void get_one(observable<nothing> &target) SILICIUM_OVERRIDE
			{
				assert(![this]() { typename ThreadingAPI::unique_lock lock(got_something_mutex); return got_something; }());
				target.async_get_one(*this);

				typename ThreadingAPI::unique_lock lock(got_something_mutex);
				while (!got_something)
				{
					got_something_set.wait(lock);
				}
				got_something = false;
			}

			virtual void got_element(nothing) SILICIUM_OVERRIDE
			{
				wake_get_one();
			}

			virtual void ended() SILICIUM_OVERRIDE
			{
				wake_get_one();
			}

			void wake_get_one()
			{
				typename ThreadingAPI::unique_lock lock(got_something_mutex);
				got_something = true;
				got_something_set.notify_one();
			}
		};

		std::unique_ptr<state_type> state;
	};

	template <class Element, class ThreadingAPI, class Action>
	auto make_thread(Action &&action) -> thread_observable<Element, ThreadingAPI>
	{
		return thread_observable<Element, ThreadingAPI>(std::forward<Action>(action));
	}
}

#endif
