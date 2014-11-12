#ifndef SILICIUM_HTTP_PARSE_RESPONSE_HPP
#define SILICIUM_HTTP_PARSE_RESPONSE_HPP

#include <silicium/noexcept_string.hpp>
#include <silicium/to_unique.hpp>
#include <silicium/detail/line_source.hpp>
#include <boost/lexical_cast.hpp>
#include <map>

namespace Si
{
	namespace http
	{
		struct response
		{
			typedef std::map<noexcept_string, noexcept_string> arguments_table;

			noexcept_string http_version;
			int status;
			noexcept_string status_text;
			std::unique_ptr<arguments_table> arguments;

			response() BOOST_NOEXCEPT
				: status(0)
			{
			}

			response(response &&other) BOOST_NOEXCEPT
				: http_version(std::move(other.http_version))
				, status(other.status)
				, status_text(std::move(other.status_text))
				, arguments(std::move(other.arguments))
			{
			}

			response(response const &other)
				: http_version(other.http_version)
				, status(other.status)
				, status_text(other.status_text)
				, arguments(other.arguments ? to_unique(*other.arguments) : nullptr)
			{
			}

			response &operator = (response &&other) BOOST_NOEXCEPT
			{
				http_version = std::move(other.http_version);
				status = std::move(other.status);
				status_text = std::move(other.status_text);
				arguments = std::move(other.arguments);
				return *this;
			}

			response &operator = (response const &other)
			{
				http_version = other.http_version;
				status = other.status;
				status_text = other.status_text;
				arguments = other.arguments ? to_unique(*other.arguments) : nullptr;
				return *this;
			}
		};

		template <class CharSource>
		boost::optional<response> parse_response(CharSource &&in)
		{
			Si::detail::line_source lines(in);
			auto first_line = get(lines);
			if (!first_line)
			{
				return boost::none;
			}
			response header;
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
	}
}

#endif
