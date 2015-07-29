#ifndef SILICIUM_OBSERVABLE_ERASE_SHARED_HPP
#define SILICIUM_OBSERVABLE_ERASE_SHARED_HPP

#include <silicium/observable/ptr.hpp>

namespace Si
{
#if SILICIUM_COMPILER_HAS_USING
	template <class Element>
	using shared_observable = ptr_observable<Element, std::shared_ptr<observable<Element, ptr_observer<observer<Element>>>>>;
#else
	template <class Element>
	struct shared_observable : ptr_observable<Element, std::shared_ptr<typename Observable<Element, ptr_observer<observer<Element>>>::interface>>
	{
		typedef ptr_observable<Element, std::shared_ptr<typename Observable<Element, ptr_observer<observer<Element>>>::interface>> base;

		template <class Initializer>
		shared_observable(Initializer &&init)
			: base(std::forward<Initializer>(init))
		{
		}
	};
#endif

	template <class Input>
	auto erase_shared(Input &&input) -> Si::shared_observable<typename std::decay<Input>::type::element_type>
	{
		typedef typename std::decay<Input>::type clean_input;
		typedef typename clean_input::element_type element_type;
		return Si::shared_observable<element_type>(
					std::make_shared<Si::virtualized_observable<clean_input, ptr_observer<observer<element_type>>>>(
						std::forward<Input>(input)
					)
				);
	}
}

#endif
