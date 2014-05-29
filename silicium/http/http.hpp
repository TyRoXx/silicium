#ifndef SILICIUM_HTTP_HTTP_HPP
#define SILICIUM_HTTP_HTTP_HPP


#include <silicium/sink.hpp>
#include <silicium/source.hpp>
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
			std::string method;
			std::string path;
			std::string http_version;
			std::map<std::string, std::string> arguments;
		};

		boost::optional<request_header> parse_header(Si::source<char> &in);

		struct response_header
		{
			std::string http_version;
			int status;
			std::string status_text;
			std::map<std::string, std::string> arguments;
		};

		void write_header(Si::sink<char> &out, response_header const &header);
	}
}

#endif
