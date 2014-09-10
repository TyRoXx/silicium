#ifndef SILICIUM_GENERATOR_OBSERVABLE_HPP
#define SILICIUM_GENERATOR_OBSERVABLE_HPP

#include <silicium/config.hpp>
#include <silicium/observer.hpp>

namespace Si
{
	template <class Generator, class Element = typename std::result_of<Generator ()>::type>
	struct generator_observable
	{
		using element_type = Element;

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
			return receiver.got_element(generate());
		}

	private:

		Generator generate;
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
