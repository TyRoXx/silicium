#ifndef SILICIUM_HTTP_HTTP_HPP
#define SILICIUM_HTTP_HTTP_HPP

#include <silicium/sink.hpp>
#include <silicium/source.hpp>
#include <silicium/noexcept_string.hpp>
#include <boost/range/algorithm/find.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <map>

namespace Si
{
	namespace http
	{
		struct request_header
		{
			noexcept_string method;
			noexcept_string path;
			noexcept_string http_version;
			std::map<noexcept_string, noexcept_string> arguments;
		};

		boost::optional<request_header> parse_header(Si::source<char> &in);
		void write_header(Si::sink<char> &out, request_header const &header);

		struct response_header
		{
			noexcept_string http_version;
			int status;
			noexcept_string status_text;
			std::unique_ptr<std::map<noexcept_string, noexcept_string>> arguments;
		};

		boost::optional<response_header> parse_response_header(Si::source<char> &in);
		void write_header(Si::sink<char> &out, response_header const &header);
	}
}

#endif
