#ifndef SILICIUM_HTTP_GENERATE_RESPONSE_HPP
#define SILICIUM_HTTP_GENERATE_RESPONSE_HPP

#include <silicium/http/generate_header.hpp>
#include <silicium/http/parse_response.hpp>

namespace Si
{
	namespace http
	{
		template <class CharSink, class Version, class Status, class StatusText>
		void generate_status_line(CharSink &&out, Version const &version,
		                          Status const &status,
		                          StatusText const &status_text)
		{
			append(out, version);
			append(out, " ");
			append(out, status);
			append(out, " ");
			append(out, status_text);
			append(out, "\r\n");
		}

		template <class CharSink>
		void generate_response(CharSink &&out, response const &header)
		{
			generate_status_line(
			    out, header.http_version,
			    boost::lexical_cast<std::string>(header.status),
			    header.status_text);
			detail::generate_header_map(out, *header.arguments);
			append(out, "\r\n");
		}
	}
}

#endif
