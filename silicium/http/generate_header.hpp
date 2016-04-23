#ifndef SILICIUM_HTTP_GENERATE_HEADER_HPP
#define SILICIUM_HTTP_GENERATE_HEADER_HPP

#include <silicium/noexcept_string.hpp>
#include <silicium/sink/sink.hpp>
#include <map>

namespace Si
{
    namespace http
    {
        template <class CharSink>
        void finish_line(CharSink &&out)
        {
            append(out, "\r\n");
        }

        template <class CharSink, class Key, class Value>
        void generate_header(CharSink &&out, Key const &key, Value const &value)
        {
            append(out, key);
            append(out, ": ");
            append(out, value);
            finish_line(out);
        }

        namespace detail
        {
            template <class CharSink>
            void generate_header_map(
                CharSink &&out,
                std::map<noexcept_string, noexcept_string> const &arguments)
            {
                for (auto const &argument : arguments)
                {
                    generate_header(out, argument.first, argument.second);
                }
            }
        }

        template <class CharSink>
        void finish_headers(CharSink &&out)
        {
            finish_line(out);
        }
    }
}

#endif
