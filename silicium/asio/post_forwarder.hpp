#ifndef SILICIUM_ASIO_POST_FORWARDER_HPP
#define SILICIUM_ASIO_POST_FORWARDER_HPP

#include <silicium/config.hpp>
#include <silicium/observable/for_each.hpp>
#include <boost/asio/io_service.hpp>

namespace Si
{
	namespace asio
	{
#if SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		template <class Observable, class Callback>
		auto make_post_forwarder(boost::asio::io_service &io, Observable &&from, Callback &&handler)
		{
			typedef typename std::decay<Observable>::type clean_observable;
			return for_each(
				std::forward<Observable>(from),
				[&io, handler = std::forward<Callback>(handler)](typename clean_observable::element_type element)
			{
				io.post([element = std::move(element), handler]() mutable
				{
					handler(std::move(element));
				});
			});
		}
#endif
	}
}

#endif
