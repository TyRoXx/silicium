#ifndef SILICIUM_HTTP_URI_HPP
#define SILICIUM_HTTP_URI_HPP

#include <silicium/iterator_range.hpp>
#include <boost/optional.hpp>
#include <uriparser/Uri.h>
#include <vector>

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

			struct uri_deleter
			{
				void operator()(UriUriA *uri) const BOOST_NOEXCEPT
				{
					assert(uri);
					uriFreeUriMembersA(uri);
				}
			};

			struct query_list_deleter
			{
				void operator()(UriQueryListA *list) const BOOST_NOEXCEPT
				{
					assert(list);
					uriFreeQueryListA(list);
				}
			};
		}

		inline boost::optional<uri> parse_uri(iterator_range<char const *> encoded)
		{
			UriParserStateA parser;
			UriUriA parsed;
			parser.uri = &parsed;
			std::unique_ptr<UriUriA, detail::uri_deleter> const parsed_clean_up(&parsed);
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

		typedef std::pair<std::string, boost::optional<std::string>> html_query_pair;

		inline boost::optional<std::vector<html_query_pair>> parse_html_query(iterator_range<char const *> encoded_query)
		{
			UriQueryListA *pairs = nullptr;
			int pair_count = 0;
			if (uriDissectQueryMallocA(&pairs, &pair_count, encoded_query.begin(), encoded_query.end()) != URI_SUCCESS)
			{
				return boost::none;
			}
			std::unique_ptr<UriQueryListA, detail::query_list_deleter> const pairs_clean_up(pairs);
			std::vector<html_query_pair> converted_pairs;
			converted_pairs.reserve(static_cast<size_t>(pair_count));
			for (UriQueryListA *p = pairs; p; p = p->next)
			{
				html_query_pair converted_pair;
				converted_pair.first = p->key;
				if (p->value)
				{
					converted_pair.second = boost::in_place(p->value);
				}
				converted_pairs.emplace_back(std::move(converted_pair));
			}
			return std::move(converted_pairs);
		}
	}
}

#endif
