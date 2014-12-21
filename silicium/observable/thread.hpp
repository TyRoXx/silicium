#ifndef SILICIUM_THREAD_HPP
#define SILICIUM_THREAD_HPP

#include <silicium/config.hpp>
#include <functional>

namespace Si
{
	template <class Element, class ThreadingAPI>
	struct thread_observable
	{
		typedef Element element_type;

		thread_observable()
		    : m_has_finished(false)
		{
		}

		explicit thread_observable(std::function<element_type ()> action)
			: m_action(std::move(action))
			, m_has_finished(false)
		{
		}

		template <class Observer>
		void async_get_one(Observer &&observer)
		{
			if (m_has_finished)
			{
				return std::forward<Observer>(observer).ended();
			}
			assert(m_action);
			auto action = std::move(m_action);
			m_worker = ThreadingAPI::launch_async([
				this,
				observer
#if SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE
					= std::forward<Observer>(observer)
#endif
				,
				action
#if SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE
					= std::move(action)
#endif
				]() mutable
			{
				m_has_finished = true;
				std::forward<Observer>(observer).got_element(action());
			});
		}

	private:

		std::function<element_type ()> m_action;
		typename ThreadingAPI::template future<void>::type m_worker;
		bool m_has_finished;
	};

	template <class ThreadingAPI, class Action>
	auto make_thread_observable(Action &&action)
	{
		return thread_observable<decltype(action()), ThreadingAPI>(std::forward<Action>(action));
	}
}

#endif
