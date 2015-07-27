#ifndef SILICIUM_HTML_TREE_HPP
#define SILICIUM_HTML_TREE_HPP

#include <silicium/html/generator.hpp>
#include <silicium/sink/iterator_sink.hpp>

namespace Si
{
	namespace html
	{
		template <std::size_t Length>
		struct exact_length
		{
		};

		template <std::size_t Length>
		struct min_length
		{
		};

		namespace detail
		{
			template <class FirstLength, class SecondLength>
			struct concatenate;

			template <std::size_t FirstLength, std::size_t SecondLength>
			struct concatenate<exact_length<FirstLength>, exact_length<SecondLength>>
			{
				typedef exact_length<FirstLength + SecondLength> type;
			};

			template <std::size_t FirstLength, std::size_t SecondLength>
			struct concatenate<min_length<FirstLength>, exact_length<SecondLength>>
			{
				typedef min_length<FirstLength + SecondLength> type;
			};

			template <std::size_t FirstLength, std::size_t SecondLength>
			struct concatenate<exact_length<FirstLength>, min_length<SecondLength>>
			{
				typedef min_length<FirstLength + SecondLength> type;
			};

			template <std::size_t FirstLength, std::size_t SecondLength>
			struct concatenate<min_length<FirstLength>, min_length<SecondLength>>
			{
				typedef min_length<FirstLength + SecondLength> type;
			};

			template <class ContentGenerator, class Length>
			struct element
			{
				typedef Length length_type;
				ContentGenerator generate;
			};

			template <class Length, class ContentGenerator>
			auto make_element(ContentGenerator &&generate)
			{
				return element<typename std::decay<ContentGenerator>::type, Length>{std::forward<ContentGenerator>(generate)};
			}
		}

		template <std::size_t NameLength, class Element>
		auto tag(char const (&name)[NameLength], Element &&content)
		{
			typedef typename detail::concatenate<
				typename std::decay<Element>::type::length_type,
				exact_length<1 + (NameLength - 1) + 1 + 2 + (NameLength - 1) + 1>
			>::type length;
			return detail::make_element<length>([
				&name,
				SILICIUM_CAPTURE_EXPRESSION(generate_content, std::move(content.generate))
			] (sink<char, success> &destination)
			{
				html::open_element(destination, name);
				generate_content(destination);
				html::close_element(destination, name);
			});
		}

		template <std::size_t Length>
		auto text(char const (&content)[Length])
		{
			typedef min_length<Length - 1> length;
			return detail::make_element<length>([content](sink<char, success> &destination)
			{
				html::write_string(destination, content);
			});
		}

		auto sequence()
		{
			return detail::make_element<exact_length<0>>([](sink<char, success> &)
			{
			});
		}

		template <class Head, class ...Tail>
		auto sequence(Head &&head, Tail &&...tail)
		{
			auto tail_elements = sequence(std::forward<Tail>(tail)...);
			typedef typename detail::concatenate<
				typename std::decay<Head>::type::length_type,
				typename decltype(tail_elements)::length_type
			>::type length;
			return detail::make_element<length>([
				SILICIUM_CAPTURE_EXPRESSION(generate_head, std::move(head.generate)),
				SILICIUM_CAPTURE_EXPRESSION(generate_tail_elements, std::move(tail_elements.generate))
				] (sink<char, success> &destination)
			{
				generate_head(destination);
				generate_tail_elements(destination);
			});
		}

		template <class Length, class ContentGenerator>
		auto dynamic(ContentGenerator &&generate)
		{
			return detail::make_element<Length>(std::forward<ContentGenerator>(generate));
		}

		template <std::size_t Length>
		auto raw(char const (&content)[Length])
		{
			return dynamic<exact_length<Length - 1>>([&content](sink<char, success> &destination)
			{
				Si::append(destination, content);
			});
		}

		namespace detail
		{
			template <class A, class B, class C, class D>
			auto operator + (element<A, B> &&left, element<C, D> &&right)
			{
				return sequence(std::move(left), std::move(right));
			}
		}

		template <class ContiguousContainer, class A, class B>
		ContiguousContainer generate(detail::element<A, B> const &tree)
		{
			ContiguousContainer generated;
			auto sink = Sink<char, success>::erase(make_container_sink(generated));
			tree.generate(sink);
			return generated;
		}
	}
}

#endif
