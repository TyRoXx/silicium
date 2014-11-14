#ifndef SILICIUM_HTML_HPP
#define SILICIUM_HTML_HPP

#include <silicium/source/source.hpp>
#include <silicium/sink/sink.hpp>

namespace Si
{
	namespace html
	{
		template <class CharSink, class Char>
		void write_char(CharSink &&sink, Char c)
		{
			using Si::append;
			switch (c)
			{
			case '&' : append(sink, "&amp;"); break;
			case '<' : append(sink, "&lt;"); break;
			case '>' : append(sink, "&gt;"); break;
			case '\'': append(sink, "&apos;"); break;
			case '"' : append(sink, "&quot;"); break;
			default:
				append(sink, c);
				break;
			}
		}

		template <class CharSink>
		void write_string(
			CharSink &&sink,
			std::string const &text)
		{
			for (auto c : text)
			{
				write_char(sink, c);
			}
		}

		template <class CharSink>
		void open_attributed_element(
			CharSink &&sink,
			std::string const &name)
		{
			append(sink, '<');
			write_string(sink, name);
		}

		template <class CharSink>
		void finish_attributes(
			CharSink &&sink)
		{
			append(sink, '>');
		}

		template <class CharSink>
		void add_attribute(
			CharSink &&sink,
			std::string const &key,
			std::string const &value)
		{
			append(sink, key);
			append(sink, "=\"");
			write_escaped(sink, value);
			append(sink, '"');
		}

		template <class CharSink>
		void open_element(
			CharSink &&sink,
			std::string const &name)
		{
			open_attributed_element(sink, name);
			finish_attributes(sink);
		}

		template <class CharSink>
		void close_element(
			CharSink &&sink,
			std::string const &name)
		{
			using Si::append;
			append(sink, '<');
			append(sink, '/');
			append(sink, name);
			append(sink, '>');
		}

		template <class CharSink>
		struct generator
		{
			generator()
			{
			}

			explicit generator(CharSink out)
				: m_out(std::move(out))
			{
			}

			template <class ContentMaker>
			void element(std::string const &name, ContentMaker make_content)
			{
				open_element(m_out, name);
				make_content();
				close_element(m_out, name);
			}

			void write(std::string const &text)
			{
				write_string(m_out, text);
			}

		private:

			CharSink m_out;
		};

		template <class CharSink>
		auto make_generator(CharSink &&sink)
			-> generator<typename std::decay<CharSink>::type>
		{
			return generator<typename std::decay<CharSink>::type>(std::forward<CharSink>(sink));
		}
	}
}

#endif
