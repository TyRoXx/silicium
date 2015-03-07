#ifndef SILICIUM_HTTP_PARSE_REQUEST_HPP
#define SILICIUM_HTTP_PARSE_REQUEST_HPP

#include <silicium/noexcept_string.hpp>
#include <silicium/source/source.hpp>
#include <silicium/detail/line_source.hpp>
#include <silicium/to_unique.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include <map>

namespace Si
{
	namespace http
	{
		struct request
		{
			noexcept_string method;
			noexcept_string path;
			noexcept_string http_version;
			std::map<noexcept_string, noexcept_string> arguments;
		};

		template <class CharSource>
		boost::optional<request> parse_request(CharSource &&in)
		{
			Si::detail::line_source lines(in);
			auto first_line = get(lines);
			if (!first_line)
			{
				return boost::none;
			}
			request header;
			{
				auto const method_end = std::find(first_line->begin(), first_line->end(), ' ');
				if (method_end == first_line->end())
				{
					return boost::none;
				}
				header.method.assign(first_line->begin(), method_end);

				auto const path_begin = method_end + 1;
				auto const path_end = std::find(path_begin, first_line->end(), ' ');
				if (path_end == first_line->end())
				{
					return boost::none;
				}
				header.path.assign(path_begin, path_end);

				header.http_version.assign(path_end + 1, first_line->end());
			}

			for (;;)
			{
				auto value_line = get(lines);
				if (!value_line)
				{
					return boost::none;
				}
				if (value_line->empty())
				{
					break;
				}
				auto value = Si::detail::split_value_line(*value_line);
				header.arguments[value.first] = std::move(value.second);
			}
			return std::move(header);
		}
	}
}

#endif
