#ifndef SILICIUM_GENERATOR_OBSERVABLE_HPP
#define SILICIUM_GENERATOR_OBSERVABLE_HPP

#include <silicium/config.hpp>
#include <silicium/observable/observer.hpp>
#include <silicium/detail/element_from_optional_like.hpp>
#include <silicium/detail/proper_value_function.hpp>

namespace Si
{
	template <class Generator,
	          class Element = typename detail::element_from_optional_like<
	              typename std::result_of<Generator()>::type>::type>
	struct generator_observable
	{
		typedef Element element_type;

		generator_observable()
		{
		}

		explicit generator_observable(Generator generate)
		    : generate(std::move(generate))
		{
		}

#if !SILICIUM_COMPILER_GENERATES_MOVES
		generator_observable(generator_observable &&other)
		    : generate(std::move(other.generate))
		{
		}

		generator_observable(generator_observable const &other)
		    : generate(other.generate)
		{
		}

		generator_observable &operator=(generator_observable &&other)
		{
			generate = std::move(other.generate);
			return *this;
		}

		generator_observable &operator=(generator_observable const &other)
		{
			generate = other.generate;
			return *this;
		}
#endif

		template <class Observer>
		void async_get_one(Observer &&receiver)
		{
			boost::optional<Element> element = generate();
			if (element)
			{
				std::forward<Observer>(receiver)
				    .got_element(std::move(*element));
			}
			else
			{
				std::forward<Observer>(receiver).ended();
			}
		}

	private:
		typedef
#if SILICIUM_DETAIL_HAS_PROPER_VALUE_FUNCTION
		    typename detail::proper_value_function<
		        Generator, typename std::result_of<Generator()>::type>::type
#else
		    Generator
#endif
		        proper_generator;

		proper_generator generate;
	};

	template <class Generator>
	auto make_generator_observable(Generator &&generate)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
	    -> generator_observable<typename std::decay<Generator>::type>
#endif
	{
		return generator_observable<typename std::decay<Generator>::type>(
		    std::forward<Generator>(generate));
	}
}

#endif
