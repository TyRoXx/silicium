#include <boost/asio/async_result.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/use_future.hpp>
#include <silicium/boost_threading.hpp>
#include <silicium/config.hpp>

namespace Si
{
	namespace asio
	{
		template <class T, class ThreadingAPI>
		struct background_task
		{
			template <class F, class CompletionToken>
			auto async_call(F &&function, CompletionToken &&token)
			{
				typedef typename boost::asio::handler_type<CompletionToken, void(T)>::type handler_type;
				handler_type handler(std::forward<CompletionToken>(token));
				boost::asio::async_result<handler_type> result(handler);
				m_handle = ThreadingAPI::launch_async([
					SILICIUM_MOVE_CAPTURE(function, std::forward<F>(function)),
					SILICIUM_MOVE_CAPTURE(handler, std::move(handler))
				]() mutable
				{
					std::move(handler)(std::forward<F>(function)());
				});
				return result.get();
			}

			~background_task()
			{
				m_handle.get();
			}

		private:

			typename ThreadingAPI::template future<void>::type m_handle;
		};
	}
}

BOOST_AUTO_TEST_CASE(background_task_async_call)
{
	Si::asio::background_task<int, Si::boost_threading> task;
	std::future<int> f = task.async_call(
		[]
		{
			return 4;
		},
		boost::asio::use_future
	);
	BOOST_CHECK_EQUAL(4, f.get());
}

BOOST_AUTO_TEST_CASE(background_task_async_call_into_coroutine)
{
	boost::thread::id const test_thread = boost::this_thread::get_id();
	boost::thread::id background_thread;
	{
		Si::asio::background_task<Si::nothing, Si::boost_threading> task;
		auto handler = [](){};
		boost::asio::spawn(
			std::move(handler),
			[&task, &background_thread, test_thread](boost::asio::basic_yield_context<decltype(handler)> yield)
		{
			BOOST_CHECK_EQUAL(test_thread, boost::this_thread::get_id());
			task.async_call(
				[]()
				{
					return Si::nothing();
				}, yield);
			//the coroutine is running the background thread now
			background_thread = boost::this_thread::get_id();
			BOOST_CHECK_NE(test_thread, background_thread);
		});
	}
	BOOST_CHECK_NE(background_thread, boost::thread::id());
	BOOST_CHECK_NE(background_thread, test_thread);
}
