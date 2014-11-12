#ifndef SILICIUM_HTTP_GENERATE_REQUEST_HPP
#define SILICIUM_HTTP_GENERATE_REQUEST_HPP

#include <silicium/http/generate_header.hpp>
#include <silicium/http/parse_request.hpp>

namespace Si
{
	namespace http
	{
		template <class CharSink, class Method, class Path, class Version>
		void write_request_line(CharSink &&out, Method const &method, Path const &path, Version const &version)
		{
			append(out, method);
			append(out, " ");
			append(out, path);
			append(out, " ");
			append(out, version);
			append(out, "\r\n");
		}

		template <class CharSink>
		void write_header(CharSink &&out, request const &header)
		{
			write_request_line(out, header.method, header.path, header.http_version);
			detail::write_arguments_map(out, header.arguments);
			append(out, "\r\n");
		}
	}
}

#endif
