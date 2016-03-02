#ifndef SILICIUM_HTML_TREE_HPP
#define SILICIUM_HTML_TREE_HPP

#include <silicium/html/generator.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <silicium/trait.hpp>
#include <silicium/identity.hpp>

#define SILICIUM_HAS_HTML_TREE                                                 \
	(SILICIUM_COMPILER_HAS_VARIADIC_TEMPLATES && !SILICIUM_GCC46)

namespace Si
{
	namespace html
	{
		typedef Sink<char, success>::interface code_sink;

#if SILICIUM_HAS_HTML_TREE
		template <std::size_t Length>
		struct exact_length : std::integral_constant<std::size_t, Length>
		{
		};

		template <std::size_t Length>
		struct min_length : std::integral_constant<std::size_t, Length>
		{
		};

		namespace detail
		{
			template <class FirstLength, class SecondLength>
			struct concatenate;

			template <std::size_t FirstLength, std::size_t SecondLength>
			struct concatenate<exact_length<FirstLength>,
			                   exact_length<SecondLength>>
			{
				typedef exact_length<FirstLength + SecondLength> type;
			};

			template <std::size_t FirstLength, std::size_t SecondLength>
			struct concatenate<min_length<FirstLength>,
			                   exact_length<SecondLength>>
			{
				typedef min_length<FirstLength + SecondLength> type;
			};

			template <std::size_t FirstLength, std::size_t SecondLength>
			struct concatenate<exact_length<FirstLength>,
			                   min_length<SecondLength>>
			{
				typedef min_length<FirstLength + SecondLength> type;
			};

			template <std::size_t FirstLength, std::size_t SecondLength>
			struct concatenate<min_length<FirstLength>,
			                   min_length<SecondLength>>
			{
				typedef min_length<FirstLength + SecondLength> type;
			};

			template <class ContentGenerator, class Length>
			struct element
			{
				typedef Length length_type;
				ContentGenerator generate;

#if SILICIUM_VC2013
				// workaround against superfluous warnings C4510 and C4610
				// ("constructor cannot be generated")
				explicit element(ContentGenerator generate)
				    : generate(std::move(generate))
				{
				}
#endif
			};

			template <class Length, class ContentGenerator>
			auto make_element(ContentGenerator &&generate)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			    -> element<typename std::decay<ContentGenerator>::type, Length>
#endif
			{
				return element<typename std::decay<ContentGenerator>::type,
				               Length>{
				    std::forward<ContentGenerator>(generate)};
			}
		}

		template <class Length = min_length<0>>
		SILICIUM_TRAIT_WITH_TYPEDEFS(Element, typedef Length length_type;
		                             , ((generate, (1, (code_sink &)), void,
		                                 const)))

		template <class ContentGenerator>
		auto dynamic(ContentGenerator &&generate)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		    -> decltype(detail::make_element<min_length<0>>(
		        std::forward<ContentGenerator>(generate)))
#endif
		{
			return detail::make_element<min_length<0>>(
			    std::forward<ContentGenerator>(generate));
		}

		template <std::size_t Length, class ContentGenerator>
		auto fixed_length(ContentGenerator &&generate)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		    -> decltype(detail::make_element<exact_length<Length>>(
		        std::forward<ContentGenerator>(generate)))
#endif
		{
			return detail::make_element<exact_length<Length>>(
			    std::forward<ContentGenerator>(generate));
		}

		template <std::size_t Length>
		auto raw(char const(&content)[Length])
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		    -> detail::element<std::function<void(code_sink &)>,
		                       exact_length<Length - 1>>
#endif
		{
			return fixed_length<Length - 1>(
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			    std::function<void(code_sink &)>
#endif
			    ([&content](code_sink &destination)
			     {
				     Si::append(destination, content);
				 }));
		}

		template <std::size_t NameLength, class Attributes, class Element,
		          class ResultLength = typename detail::concatenate<
		              typename std::decay<Element>::type::length_type,
		              typename detail::concatenate<
		                  typename std::decay<Attributes>::type::length_type,
		                  exact_length<1 + (NameLength - 1) + 1 + 2 +
		                               (NameLength - 1) + 1>>::type>::type>
		auto tag(char const(&name)[NameLength], Attributes &&attributes,
		         Element &&content)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		    -> detail::element<std::function<void(code_sink &)>, ResultLength>
#endif
		{
			auto &generate_content = content.generate;
			return detail::make_element<ResultLength>(
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			    std::function<void(code_sink &)>
#endif
			    ([
				   &name,
				   SILICIUM_CAPTURE_EXPRESSION(
				       generate_content, std::move(generate_content)),
				   SILICIUM_CAPTURE_EXPRESSION(
				       attributes, std::forward<Attributes>(attributes))
				](code_sink & destination)
			     {
				     html::open_attributed_element(destination, name);
				     attributes.generate(destination);
				     html::finish_attributes(destination);
				     generate_content(destination);
				     html::close_element(destination, name);
				 }));
		}

		template <std::size_t NameLength, class Attributes,
		          class ResultLength = typename detail::concatenate<
		              min_length<1 + (NameLength - 1) + 2>,
		              typename std::decay<Attributes>::type::length_type>::type>
		auto tag(char const(&name)[NameLength], Attributes &&attributes,
		         empty_t)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		    -> detail::element<std::function<void(code_sink &)>, ResultLength>
#endif
		{
			return detail::make_element<ResultLength>(
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			    std::function<void(code_sink &)>
#endif
			    ([
				   &name,
				   SILICIUM_CAPTURE_EXPRESSION(
				       attributes, std::forward<Attributes>(attributes))
				](code_sink & destination)
			     {
				     html::open_attributed_element(destination, name);
				     attributes.generate(destination);
				     finish_attributes_of_unpaired_tag(destination);
				 }));
		}

		namespace detail
		{
			inline void no_attributes(code_sink &)
			{
			}
		}

		template <std::size_t NameLength, class Element>
		auto tag(char const(&name)[NameLength], Element &&content)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		    -> decltype(tag(name, detail::make_element<exact_length<0>>(
		                              &detail::no_attributes),
		                    std::forward<Element>(content)))
#endif
		{
			return tag(name, detail::make_element<exact_length<0>>(
			                     &detail::no_attributes),
			           std::forward<Element>(content));
		}

		namespace detail
		{
			static BOOST_CONSTEXPR_OR_CONST std::size_t space = 1;
			static BOOST_CONSTEXPR_OR_CONST std::size_t assign = 1;
			static BOOST_CONSTEXPR_OR_CONST std::size_t quote = 1;
		}

		template <std::size_t KeyLength, std::size_t ValueLength,
		          std::size_t ResultLength = detail::space + (KeyLength - 1) +
		                                     detail::assign + detail::quote +
		                                     (ValueLength - 1) + detail::quote>
		auto attribute(char const(&key)[KeyLength],
		               char const(&value)[ValueLength])
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		    -> detail::element<std::function<void(code_sink &)>,
		                       min_length<ResultLength>>
#endif
		{
			return detail::make_element<min_length<ResultLength>>(
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			    std::function<void(code_sink &)>
#endif
			    ([&key, &value](code_sink &destination)
			     {
				     add_attribute(destination, key, value);
				 }));
		}

		template <std::size_t KeyLength,
		          std::size_t ResultLength = detail::space + (KeyLength - 1) +
		                                     detail::assign + detail::quote +
		                                     detail::quote>
		auto attribute(char const(&key)[KeyLength], std::string value)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		    -> detail::element<std::function<void(code_sink &)>,
		                       min_length<ResultLength>>
#endif
		{
			return detail::make_element<min_length<ResultLength>>(
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			    std::function<void(code_sink &)>
#endif
			    ([&key, SILICIUM_CAPTURE_EXPRESSION(value, std::move(value)) ](
			        code_sink & destination)
			     {
				     add_attribute(destination, key, value);
				 }));
		}

		template <std::size_t KeyLength,
		          std::size_t ResultLength = detail::space + (KeyLength - 1)>
		auto attribute(char const(&key)[KeyLength])
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		    -> detail::element<std::function<void(code_sink &)>,
		                       min_length<ResultLength>>
#endif
		{
			return detail::make_element<min_length<ResultLength>>(
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			    std::function<void(code_sink &)>
#endif
			    ([&key](code_sink &destination)
			     {
				     add_attribute(destination, key);
				 }));
		}

		template <std::size_t Length>
		auto text(char const(&content)[Length])
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		    -> detail::element<std::function<void(code_sink &)>,
		                       min_length<Length - 1>>
#endif
		{
			return detail::make_element<min_length<Length - 1>>(
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			    std::function<void(code_sink &)>
#endif
			    ([&content](code_sink &destination)
			     {
				     html::write_string(destination, content);
				 }));
		}

		inline auto text(std::string content)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		    -> detail::element<std::function<void(code_sink &)>, min_length<0>>
#endif
		{
			return detail::make_element<min_length<0>>(
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			    std::function<void(code_sink &)>
#endif
			    ([SILICIUM_CAPTURE_EXPRESSION(content, std::move(content))](
			        code_sink & destination)
			     {
				     html::write_string(destination, content);
				 }));
		}

		namespace detail
		{
			inline void no_content(code_sink &)
			{
			}
		}

		inline auto sequence()
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		    -> decltype(
		        detail::make_element<exact_length<0>>(&detail::no_content))
#endif
		{
			return detail::make_element<exact_length<0>>(&detail::no_content);
		}

		template <class Head, class... Tail>
		auto sequence(Head &&head, Tail &&... tail)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		    -> detail::element<
		        std::function<void(code_sink &)>,
		        typename detail::concatenate<
		            typename std::decay<Head>::type::length_type,
		            typename identity<decltype(sequence(std::forward<Tail>(
		                tail)...))>::type::length_type>::type>
#endif
		{
			auto tail_elements = sequence(std::forward<Tail>(tail)...);
			auto &generate_head = head.generate;
			auto &generate_tail_elements = tail_elements.generate;
			return detail::make_element<typename detail::concatenate<
			    typename std::decay<Head>::type::length_type,
			    typename decltype(tail_elements)::length_type>::type>(
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			    std::function<void(code_sink &)>
#endif
			    ([
				   SILICIUM_CAPTURE_EXPRESSION(
				       generate_head, std::move(generate_head)),
				   SILICIUM_CAPTURE_EXPRESSION(
				       generate_tail_elements,
				       std::move(generate_tail_elements))
				](code_sink & destination)
			     {
				     generate_head(destination);
				     generate_tail_elements(destination);
				 }));
		}

		namespace detail
		{
			template <class A, class B, class C, class D>
			auto operator+(element<A, B> &&left, element<C, D> &&right)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			    -> decltype(sequence(std::move(left), std::move(right)))
#endif
			{
				return sequence(std::move(left), std::move(right));
			}
		}

		template <class ContiguousContainer, class A, class B>
		ContiguousContainer generate(detail::element<A, B> const &tree)
		{
			ContiguousContainer generated;
			auto sink =
			    Sink<char, success>::erase(make_container_sink(generated));
			tree.generate(sink);
			return generated;
		}
#endif
	}
}

#endif
