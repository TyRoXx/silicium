#ifndef SILICIUM_GENERATOR_OBSERVABLE_HPP
#define SILICIUM_GENERATOR_OBSERVABLE_HPP

#include <silicium/config.hpp>
#include <silicium/observer.hpp>
#include <silicium/detail/element_from_optional_like.hpp>
#include <silicium/detail/proper_value_function.hpp>

namespace Si
{
	template <class Generator, class Element = typename detail::element_from_optional_like<typename std::result_of<Generator ()>::type>::type>
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

		generator_observable &operator = (generator_observable &&other)
		{
			generate = std::move(other.generate);
			return *this;
		}
#endif

		void async_get_one(observer<Element> &receiver)
		{
			boost::optional<Element> element = generate();
			if (element)
			{
				receiver.got_element(std::move(*element));
			}
			else
			{
				receiver.ended();
			}
		}

	private:

		typedef typename detail::proper_value_function<Generator, typename std::result_of<Generator ()>::type>::type proper_generator;

		proper_generator generate;
	};

	template <class Generator>
	auto make_generator_observable(Generator &&generate)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> generator_observable<typename std::decay<Generator>::type>
#endif
	{
		return generator_observable<typename std::decay<Generator>::type>(std::forward<Generator>(generate));
	}
}

#endif
