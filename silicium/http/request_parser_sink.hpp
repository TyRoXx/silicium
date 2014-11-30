#ifndef SILICIUM_HTTP_REQUEST_PARSER_SINK_HPP
#define SILICIUM_HTTP_REQUEST_PARSER_SINK_HPP

#include <silicium/http/parse_request.hpp>
#include <silicium/sink/sink.hpp>
#include <silicium/fast_variant.hpp>
#include <silicium/exchange.hpp>

namespace Si
{
	namespace http
	{
		template <class Output>
		struct request_parser_sink : sink<char, typename Output::error_type>
		{
			typedef char element_type;
			typedef typename Output::error_type error_type;

			explicit request_parser_sink(Output output)
				: m_output(std::move(output))
				, m_state(state::method)
			{
			}

			virtual error_type append(iterator_range<element_type const *> data) SILICIUM_OVERRIDE
			{
				while (!data.empty())
				{
					switch (m_state)
					{
					case state::method:
					case state::path:
						{
							auto space = std::find(data.begin(), data.end(), ' ');
							noexcept_string &buffer = (m_state == state::method) ? m_result.method : m_result.path;
							buffer.insert(buffer.end(), data.begin(), space);
							data.pop_front(std::distance(data.begin(), space));
							if (space != data.end())
							{
								data.pop_front(1);
								m_state = (m_state == state::method) ? state::path : state::version;
							}
							break;
						}

					case state::version:
						{
							auto cr = std::find(data.begin(), data.end(), '\r');
							noexcept_string &buffer = m_result.http_version;
							buffer.insert(buffer.end(), data.begin(), cr);
							data.pop_front(std::distance(data.begin(), cr));
							if (cr != data.end())
							{
								data.pop_front(1);
								m_state = state::version_lf;
							}
							break;
						}

					case state::version_lf:
						{
							if (data.front() == '\n')
							{
								data.pop_front(1);
							}
							m_state = state::key;
							break;
						}

					case state::key:
						{
							if (data.front() == '\r')
							{
								data.pop_front();
								m_state = state::end_of_headers_lf;
								//TODO: handle error
								m_output.append(Si::make_iterator_range(&m_result, &m_result + 1));
								m_result.path.clear();
								m_result.method.clear();
								m_result.http_version.clear();
								m_result.arguments.clear();
								break;
							}
							auto colon = std::find(data.begin(), data.end(), ':');
							m_key.insert(m_key.end(), data.begin(), colon);
							data.pop_front(std::distance(data.begin(), colon));
							if (colon != data.end())
							{
								data.pop_front(1);
								m_state = state::value_space;
							}
							break;
						}

					case state::value_space:
						{
							if (data.front() == ' ')
							{
								data.pop_front(1);
							}
							m_state = state::value;
							break;
						}

					case state::value:
						{
							auto cr = std::find(data.begin(), data.end(), '\r');
							m_value.insert(m_value.end(), data.begin(), cr);
							data.pop_front(std::distance(data.begin(), cr));
							if (cr != data.end())
							{
								data.pop_front(1);
								m_result.arguments.insert(std::make_pair(std::move(m_key), std::move(m_value)));
								m_key.clear();
								m_value.clear();
								m_state = state::value_lf;
							}
							break;
						}

					case state::value_lf:
						{
							if (data.front() == '\n')
							{
								data.pop_front(1);
							}
							m_state = state::key;
							break;
						}

					case state::end_of_headers_lf:
						{
							if (data.front() == '\n')
							{
								data.pop_front(1);
							}
							m_state = state::method;
							break;
						}
					}
				}
				return error_type();
			}

		private:

			enum class state
			{
				method,
				path,
				version,
				version_lf,
				key,
				value_space,
				value,
				value_lf,
				end_of_headers_lf
			};

			Output m_output;
			request m_result;
			state m_state;
			noexcept_string m_key;
			noexcept_string m_value;
		};

		template <class Output>
		auto make_request_parser_sink(Output &&output)
		{
			return request_parser_sink<typename std::remove_reference<Output>::type>(std::forward<Output>(output));
		}
	}
}

#endif
