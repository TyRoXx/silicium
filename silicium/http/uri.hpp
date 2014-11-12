#ifndef SILICIUM_HTTP_URI_HPP
#define SILICIUM_HTTP_URI_HPP

#include <silicium/iterator_range.hpp>
#include <boost/optional.hpp>
#include <uriparser/Uri.h>

namespace Si
{
	namespace http
	{
		struct uri
		{
			typedef iterator_range<char const *> string;

			//"http"
			string scheme;

			//"/"
			std::vector<string> path;

			//"?a=1&b=2"
			string query;

			//"#heading"
			string fragment;
		};

		namespace detail
		{
			inline uri::string string_from_parser(UriTextRangeA parsed)
			{
				return uri::string(parsed.first, parsed.afterLast);
			}
		}

		inline boost::optional<uri> parse_uri(iterator_range<char const *> encoded)
		{
			UriParserStateA parser;
			UriUriA parsed;
			parser.uri = &parsed;
			int const rc = uriParseUriExA(&parser, encoded.begin(), encoded.end());
			if (rc != URI_SUCCESS)
			{
				return boost::none;
			}
			uri result;
			result.scheme = detail::string_from_parser(parsed.scheme);
			result.query = detail::string_from_parser(parsed.query);
			result.fragment = detail::string_from_parser(parsed.fragment);
			for (auto i = parsed.pathHead; i; i = i->next)
			{
				result.path.emplace_back(detail::string_from_parser(i->text));
			}
			return result;
		}
	}
}

#endif
