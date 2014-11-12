#ifndef SILICIUM_HTTP_GENERATE_HEADER_HPP
#define SILICIUM_HTTP_GENERATE_HEADER_HPP

#include <silicium/noexcept_string.hpp>
#include <silicium/sink/sink.hpp>
#include <map>

namespace Si
{
	namespace http
	{
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
	}
}

#endif
