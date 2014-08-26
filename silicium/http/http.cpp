#include <silicium/http/http.hpp>

namespace Si
{
	namespace http
	{
		namespace detail
		{
			template <class CharRange>
			std::pair<std::string, std::string> split_value_line(CharRange const &line)
			{
				auto colon = boost::range::find(line, ':');
				auto second_begin = colon + 1;
				if (*second_begin == ' ')
				{
					++second_begin;
				}
				return std::make_pair(std::string(begin(line), colon), std::string(second_begin, end(line)));
			}
		}

		boost::optional<request_header> parse_header(Si::source<char> &in)
		{
			Si::line_source lines(in);
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

		namespace
		{
			void write_arguments_map(Si::sink<char> &out, std::map<std::string, std::string> const &arguments)
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

		void write_header(Si::sink<char> &out, request_header const &header)
		{
			append(out, header.method);
			append(out, " ");
			append(out, header.path);
			append(out, " ");
			append(out, header.http_version);
			append(out, "\r\n");
			write_arguments_map(out, header.arguments);
			append(out, "\r\n");
		}

		void write_header(Si::sink<char> &out, response_header const &header)
		{
			append(out, header.http_version);
			append(out, " ");
			append(out, boost::lexical_cast<std::string>(header.status));
			append(out, " ");
			append(out, header.status_text);
			append(out, "\r\n");
			write_arguments_map(out, header.arguments);
			append(out, "\r\n");
		}
	}
}
