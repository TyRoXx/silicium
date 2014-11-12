#ifndef SILICIUM_HTTP_HTTP_HPP
#define SILICIUM_HTTP_HTTP_HPP

#include <silicium/http/parse_response.hpp>
#include <silicium/http/parse_request.hpp>
#include <silicium/sink/sink.hpp>
#include <silicium/source/source.hpp>
#include <silicium/noexcept_string.hpp>
#include <silicium/detail/line_source.hpp>
#include <silicium/config.hpp>
#include <silicium/to_unique.hpp>
#include <boost/range/algorithm/find.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <map>

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

		template <class CharSink, class Key, class Value>
		void write_argument(CharSink &&out, Key const &key, Value const &value)
		{
			append(out, key);
			append(out, ": ");
			append(out, value);
			append(out, "\r\n");
		}

		namespace detail
		{
			template <class CharSink>
			void write_arguments_map(CharSink &&out, std::map<noexcept_string, noexcept_string> const &arguments)
			{
				for (auto const &argument : arguments)
				{
					write_argument(out, argument.first, argument.second);
				}
			}
		}

		template <class CharSink>
		void write_header(CharSink &&out, request const &header)
		{
			write_request_line(out, header.method, header.path, header.http_version);
			detail::write_arguments_map(out, header.arguments);
			append(out, "\r\n");
		}

		template <class CharSink, class Version, class Status, class StatusText>
		void write_status_line(CharSink &&out, Version const &version, Status const &status, StatusText const &status_text)
		{
			append(out, version);
			append(out, " ");
			append(out, status);
			append(out, " ");
			append(out, status_text);
			append(out, "\r\n");
		}

		template <class CharSink>
		void write_header(CharSink &&out, response const &header)
		{
			write_status_line(out, header.http_version, boost::lexical_cast<std::string>(header.status), header.status_text);
			detail::write_arguments_map(out, *header.arguments);
			append(out, "\r\n");
		}
	}
}

#endif
