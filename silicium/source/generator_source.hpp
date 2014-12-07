#ifndef SILICIUM_GENERATOR_SOURCE_HPP
#define SILICIUM_GENERATOR_SOURCE_HPP

#include <silicium/source/source.hpp>
#include <silicium/config.hpp>
#include <silicium/optional.hpp>
#include <silicium/detail/element_from_optional_like.hpp>
#include <silicium/detail/proper_value_function.hpp>

namespace Si
{
	template <class Generator, class Element = typename detail::element_from_optional_like<typename std::result_of<Generator ()>::type>::type>
	struct generator_source
	{
		typedef Element element_type;

		generator_source()
		{
		}

		explicit generator_source(Generator generate_next)
			: m_generate_next(std::move(generate_next))
		{
		}

		iterator_range<Element const *> map_next(std::size_t size)
		{
			(void)size;
			return iterator_range<Element const *>();
		}

		Element *copy_next(iterator_range<Element *> destination)
		{
			auto copied = destination.begin();
			for (; copied != destination.end(); ++copied)
			{
				auto generated = m_generate_next();
				if (!generated)
				{
					break;
				}
				*copied = std::move(*generated);
			}
			return copied;
		}

		boost::uintmax_t minimum_size()
		{
			return 0;
		}

		boost::optional<boost::uintmax_t> maximum_size()
		{
			return boost::none;
		}

		std::size_t skip(std::size_t count)
		{
			std::size_t i = 0;
			for (; i < count; ++i)
			{
				if (!m_generate_next())
				{
					break;
				}
			}
			return i;
		}

	private:

		typedef typename detail::proper_value_function<Generator, Si::optional<Element>>::type proper_generator;

		proper_generator m_generate_next;
	};

	template <class Generator>
	auto make_generator_source(Generator &&generate_next)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> generator_source<typename std::decay<Generator>::type>
#endif
	{
		return generator_source<typename std::decay<Generator>::type>{std::forward<Generator>(generate_next)};
	}

	template <class OneShotGenerator>
	auto make_oneshot_generator_source(OneShotGenerator &&generate_one)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> generator_source<std::function<Si::optional<decltype(generate_one())>()>>
#endif
	{
		bool has_generated = false;
		return make_generator_source(
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			std::function<Si::optional<decltype(generate_one())>()>
#endif
			([has_generated, generate_one
#if SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE
				= std::forward<OneShotGenerator>(generate_one)
#endif
			]() mutable -> Si::optional<decltype(generate_one())>
		{
			if (has_generated)
			{
				return Si::none;
			}
			has_generated = true;
			return std::forward<OneShotGenerator>(generate_one)();
		}));
	}
}

#endif
