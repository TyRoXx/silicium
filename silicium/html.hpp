#ifndef SILICIUM_HTML_HPP
#define SILICIUM_HTML_HPP

#include <silicium/source/source.hpp>
#include <silicium/sink/sink.hpp>
#include <silicium/config.hpp>

#if BOOST_VERSION >= 105300
#include <boost/utility/string_ref.hpp>
#else
#include <silicium/noexcept_string.hpp>
#endif

namespace Si
{
	template <class CharRange>
	auto make_range_from_string_like(CharRange &&range)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> CharRange
#endif
	{
		return std::forward<CharRange>(range);
	}

	template <class Char>
	auto make_range_from_string_like(Char const *c_str)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> decltype(make_c_str_range(c_str))
#endif
	{
		return make_c_str_range(c_str);
	}

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

		template <class CharSink, class StringLike>
		void write_string(
			CharSink &&sink,
			StringLike const &text)
		{
			for (auto c : make_range_from_string_like(text))
			{
				write_char(sink, c);
			}
		}

		template <class CharSink, class StringLike>
		void open_attributed_element(
			CharSink &&sink,
			StringLike const &name)
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

		template <class CharSink, class KeyStringLike, class ValueStringLike>
		void add_attribute(
			CharSink &&sink,
			KeyStringLike const &key,
			ValueStringLike const &value)
		{
			append(sink, " ");
			append(sink, key);
			append(sink, "=\"");
			write_string(sink, value);
			append(sink, '"');
		}

		template <class CharSink, class StringLike>
		void open_element(
			CharSink &&sink,
			StringLike const &name)
		{
			open_attributed_element(sink, name);
			finish_attributes(sink);
		}

		template <class CharSink, class StringLike>
		void close_element(
			CharSink &&sink,
			StringLike const &name)
		{
			using Si::append;
			append(sink, '<');
			append(sink, '/');
			append(sink, name);
			append(sink, '>');
		}

		template <class CharSink, class StringLike>
		void unpaired_element(
			CharSink &&sink,
			StringLike const &name)
		{
			using Si::append;
			append(sink, '<');
			append(sink, name);
			append(sink, '/');
			append(sink, '>');
		}

		struct empty_t {};

		static BOOST_CONSTEXPR_OR_CONST empty_t empty;

		template <class CharSink>
		struct generator
		{
			typedef
#if BOOST_VERSION >= 105300
				boost::string_ref
#else
				noexcept_string
#endif
				name_type;

			generator()
			{
			}

			explicit generator(CharSink out)
				: m_out(std::move(out))
			{
			}

			template <class ContentMaker>
			void element(name_type const &name, ContentMaker make_content)
			{
				open_element(m_out, name);
				make_content();
				close_element(m_out, name);
			}

			void element(name_type const &name, empty_t)
			{
				unpaired_element(m_out, name);
			}

			template <class AttributeMaker, class ContentMaker>
			void element(name_type const &name, AttributeMaker make_attributes, ContentMaker make_content)
			{
				open_attributed_element(m_out, name);
				make_attributes();
				finish_attributes(m_out);
				make_content();
				close_element(m_out, name);
			}

			template <class AttributeMaker>
			void element(name_type const &name, AttributeMaker make_attributes, empty_t)
			{
				open_attributed_element(m_out, name);
				make_attributes();
				using Si::append;
				append(m_out, "/>");
			}

			template <class KeyStringLike, class ValueStringLike>
			void attribute(
				KeyStringLike const &key,
				ValueStringLike const &value)
			{
				add_attribute(m_out, key, value);
			}

			void element(name_type const &name)
			{
				unpaired_element(m_out, name);
			}

			template <class ...Args>
			void operator()(Args &&...args)
			{
				element(std::forward<Args>(args)...);
			}

			template <class StringLike>
			void element_with_text(name_type const &name, StringLike const &text)
			{
				open_element(m_out, name);
				write(text);
				close_element(m_out, name);
			}

			template <class StringLike>
			void write(StringLike const &text)
			{
				write_string(m_out, text);
			}

			template <class StringLike>
			void raw(StringLike const &text)
			{
				append(m_out, text);
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

		namespace detail
		{
			template <class ContentGenerator>
			struct element
			{
				ContentGenerator generate;
				std::size_t min_length;
			};

			template <class ContentGenerator>
			auto make_element(ContentGenerator &&generate, std::size_t min_length)
			{
				return element<typename std::decay<ContentGenerator>::type>{std::forward<ContentGenerator>(generate), min_length};
			}
		}

		template <std::size_t NameLength, class Element>
		auto tag(char const (&name)[NameLength], Element &&content)
		{
			std::size_t const content_min_length = content.min_length;
			return detail::make_element([
				&name,
				SILICIUM_CAPTURE_EXPRESSION(generate_content, std::move(content.generate))
			] (sink<char, success> &destination)
			{
				html::open_element(destination, name);
				generate_content(destination);
				html::close_element(destination, name);
			}, 1 + (NameLength - 1) + 1 + content_min_length + 2 + (NameLength - 1) + 1);
		}

		auto text(char const *content)
		{
			return detail::make_element([content](sink<char, success> &destination)
			{
				html::write_string(destination, content);
			}, 0);
		}

		auto sequence()
		{
			return detail::make_element([](sink<char, success> &)
			{
			}, 0);
		}

		template <class Head, class ...Tail>
		auto sequence(Head &&head, Tail &&...tail)
		{
			auto tail_elements = sequence(std::forward<Tail>(tail)...);
			return detail::make_element([
				SILICIUM_CAPTURE_EXPRESSION(generate_head, std::move(head.generate)),
				SILICIUM_CAPTURE_EXPRESSION(generate_tail_elements, std::move(tail_elements.generate))
				] (sink<char, success> &destination)
			{
				generate_head(destination);
				generate_tail_elements(destination);
			}, head.min_length + tail_elements.min_length);
		}

		template <class ContentGenerator>
		auto dynamic(ContentGenerator &&generate, std::size_t min_size = 0)
		{
			return detail::make_element(std::forward<ContentGenerator>(generate), min_size);
		}

		template <std::size_t Length>
		auto raw(char const (&content)[Length])
		{
			return dynamic([&content](sink<char, success> &destination)
			{
				Si::append(destination, content);
			}, (Length - 1));
		}

		namespace detail
		{
			template <class ContentGenerator1, class ContentGenerator2>
			auto operator + (element<ContentGenerator1> &&left, element<ContentGenerator2> &&right)
			{
				return sequence(std::move(left), std::move(right));
			}
		}
	}
}

#endif
