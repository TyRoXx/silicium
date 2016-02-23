#ifndef SILICIUM_ASIO_ASYNC_HPP
#define SILICIUM_ASIO_ASYNC_HPP

#include <boost/asio/async_result.hpp>
#include <silicium/to_unique.hpp>

namespace Si
{
	namespace asio
	{
		template <class ForegroundDispatcher, class BackgroundDispatcher,
		          class NullaryFunction, class ResultHandler>
		auto async(ForegroundDispatcher &foreground,
		           BackgroundDispatcher &background, NullaryFunction &&work,
		           ResultHandler &&handle_result) ->
		    typename boost::asio::async_result<
		        typename boost::asio::handler_type<
		            ResultHandler, void(decltype(work()))>::type>::type
		{
			typedef decltype(work()) result_type;
			typename boost::asio::handler_type<ResultHandler,
			                                   void(result_type)>::type
			    real_handler(std::forward<ResultHandler>(handle_result));
			typename boost::asio::async_result<decltype(real_handler)> result(
			    real_handler);
			background.post(
			    [
				  SILICIUM_CAPTURE_EXPRESSION(
				      work, std::forward<NullaryFunction>(work)),
				  SILICIUM_CAPTURE_EXPRESSION(
				      real_handler, std::move(real_handler)),
				  &foreground
				]() mutable
			    {
				    // unfortunately a post()ed function must be copyable
				    // currently, therefore the
				    // shared_ptr
				    std::shared_ptr<result_type> result = Si::to_unique(work());
				    foreground.post([
					    SILICIUM_CAPTURE_EXPRESSION(result, std::move(result)),
					    SILICIUM_CAPTURE_EXPRESSION(
					        real_handler, std::move(real_handler))
					]() mutable
				                    {
					                    real_handler(std::move(*result));
					                });
				});
			return result.get();
		}
	}
}

#endif
