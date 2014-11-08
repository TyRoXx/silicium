#ifndef SILICIUM_HTTP_HTTP_HPP
#define SILICIUM_HTTP_HTTP_HPP

#include <silicium/sink/sink.hpp>
#include <silicium/source.hpp>
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
		struct request_header
		{
			noexcept_string method;
			noexcept_string path;
			noexcept_string http_version;
			std::map<noexcept_string, noexcept_string> arguments;
		};

		namespace detail
		{
			template <class CharRange>
			std::pair<noexcept_string, noexcept_string> split_value_line(CharRange const &line)
			{
				auto colon = boost::range::find(line, ':');
				auto second_begin = colon + 1;
				if (*second_begin == ' ')
				{
					++second_begin;
				}
				return std::make_pair(noexcept_string(begin(line), colon), noexcept_string(second_begin, end(line)));
			}
		}

		template <class CharSource>
		boost::optional<request_header> parse_header(CharSource &&in)
		{
			Si::detail::line_source lines(in);
			auto first_line = get(lines);
			if (!first_line)
			{
				return boost::none;
			}
			request_header header;
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
				auto value = detail::split_value_line(*value_line);
				header.arguments[value.first] = std::move(value.second);
			}
			return std::move(header);
		}

		namespace detail
		{
			template <class CharSink>
			void write_arguments_map(CharSink &&out, std::map<noexcept_string, noexcept_string> const &arguments)
			{
				for (auto const &argument : arguments)
				{
					append(out, argument.first);
					append(out, ": ");
					append(out, argument.second);
					append(out, "\r\n");
				}
			}
		}

		template <class CharSink>
		void write_header(CharSink &&out, request_header const &header)
		{
			append(out, header.method);
			append(out, " ");
			append(out, header.path);
			append(out, " ");
			append(out, header.http_version);
			append(out, "\r\n");
			detail::write_arguments_map(out, header.arguments);
			append(out, "\r\n");
		}

		struct response_header
		{
			noexcept_string http_version;
			int status;
			noexcept_string status_text;
			std::unique_ptr<std::map<noexcept_string, noexcept_string>> arguments;

			response_header() BOOST_NOEXCEPT
			{
			}

			response_header(response_header &&other) BOOST_NOEXCEPT
				: http_version(std::move(other.http_version))
				, status(other.status)
				, status_text(std::move(other.status_text))
				, arguments(std::move(other.arguments))
			{
			}

			response_header(response_header const &other)
				: http_version(other.http_version)
				, status(other.status)
				, status_text(other.status_text)
				, arguments(other.arguments ? to_unique(*other.arguments) : nullptr)
			{
			}

			response_header &operator = (response_header &&other) BOOST_NOEXCEPT
			{
				http_version = std::move(other.http_version);
				status = std::move(other.status);
				status_text = std::move(other.status_text);
				arguments = std::move(other.arguments);
				return *this;
			}

			response_header &operator = (response_header const &other)
			{
				http_version = other.http_version;
				status = other.status;
				status_text = other.status_text;
				arguments = other.arguments ? to_unique(*other.arguments) : nullptr;
				return *this;
			}
		};

		template <class CharSource>
		boost::optional<response_header> parse_response_header(CharSource &&in)
		{
			Si::detail::line_source lines(in);
			auto first_line = get(lines);
			if (!first_line)
			{
				return boost::none;
			}
			response_header header;
			header.arguments = Si::make_unique<std::map<noexcept_string, noexcept_string>>();
			{
				auto const version_end = std::find(first_line->begin(), first_line->end(), ' ');
				if (version_end == first_line->end())
				{
					return boost::none;
				}
				header.http_version.assign(first_line->begin(), version_end);

				auto const status_begin = version_end + 1;
				auto const status_end = std::find(status_begin, first_line->end(), ' ');
				if (status_end == first_line->end())
				{
					return boost::none;
				}
				try
				{
					header.status = boost::lexical_cast<unsigned>(std::string(status_begin, status_end));
				}
				catch (boost::bad_lexical_cast const &)
				{
					return boost::none;
				}

				header.status_text.assign(status_end + 1, first_line->end());
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
				auto value = detail::split_value_line(*value_line);
				(*header.arguments)[value.first] = std::move(value.second);
			}
			return std::move(header);
		}

		template <class CharSink>
		void write_header(CharSink &&out, response_header const &header)
		{
			append(out, header.http_version);
			append(out, " ");
			append(out, boost::lexical_cast<std::string>(header.status));
			append(out, " ");
			append(out, header.status_text);
			append(out, "\r\n");
			detail::write_arguments_map(out, *header.arguments);
			append(out, "\r\n");
		}
	}
}

#endif
