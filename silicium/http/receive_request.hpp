#ifndef SILICIUM_HTTP_RECEIVE_REQUEST_HPP
#define SILICIUM_HTTP_RECEIVE_REQUEST_HPP

#include <silicium/asio/reading_observable.hpp>
#include <silicium/source/error_extracting_source.hpp>
#include <silicium/source/enumerating_source.hpp>
#include <silicium/source/observable_source.hpp>
#include <silicium/source/ref.hpp>
#include <silicium/http/parse_request.hpp>
#include <boost/array.hpp>

namespace Si
{
	namespace http
	{
		template <class AsyncStream, class YieldContext>
		Si::error_or<boost::optional<Si::http::request>> receive_request(AsyncStream &client, YieldContext &&yield)
		{
			std::array<char, 4096> incoming_buffer;
			auto receiver = Si::asio::make_reading_observable(client, Si::make_memory_range(incoming_buffer));
			auto without_errors = Si::make_error_extracting_source(Si::make_observable_source(std::move(receiver), yield));
			auto enumerated = Si::make_enumerating_source(Si::ref_source(without_errors));
			auto request = Si::http::parse_request(enumerated);
			if (without_errors.get_last_error())
			{
				return without_errors.get_last_error();
			}
			return request;
		}
	}
}

#endif
