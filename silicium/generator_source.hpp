#ifndef SILICIUM_GENERATOR_SOURCE_HPP
#define SILICIUM_GENERATOR_SOURCE_HPP

#include <silicium/source.hpp>
#include <silicium/config.hpp>
#include <silicium/detail/proper_value_function.hpp>

namespace Si
{
	namespace detail
	{
		template <class Element>
		struct element_from_optional_like
		{
			using type = Element;
		};

		template <class Element>
		struct element_from_optional_like<boost::optional<Element>>
		{
			using type = Element;
		};
	}

	template <class Generator, class Element = typename detail::element_from_optional_like<typename std::result_of<Generator ()>::type>::type>
	struct generator_source
	{
		using element_type = Element;

		generator_source()
		{
		}

		explicit generator_source(Generator generate_next)
			: m_generate_next(std::move(generate_next))
		{
		}

		boost::iterator_range<Element const *> map_next(std::size_t size)
		{
			(void)size;
			return boost::iterator_range<Element const *>();
		}

		Element *copy_next(boost::iterator_range<Element *> destination)
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

		using proper_generator = typename detail::proper_value_function<Generator, boost::optional<Element>>::type;

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
}

#endif
