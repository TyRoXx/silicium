#ifndef SILICIUM_OBSERVABLE_ERASE_UNIQUE_HPP
#define SILICIUM_OBSERVABLE_ERASE_UNIQUE_HPP

#include <silicium/observable/ptr.hpp>

namespace Si
{
#if SILICIUM_COMPILER_HAS_USING
	template <class Element>
	using unique_observable =
	    ptr_observable<Element, std::unique_ptr<observable<Element, ptr_observer<observer<Element>>>>>;
#else
	template <class Element>
	struct unique_observable
	    : ptr_observable<Element,
	                     std::unique_ptr<typename Observable<Element, ptr_observer<observer<Element>>>::interface>>
	{
		typedef ptr_observable<
		    Element, std::unique_ptr<typename Observable<Element, ptr_observer<observer<Element>>>::interface>> base;

		template <class Initializer>
		unique_observable(Initializer &&init)
		    : base(std::forward<Initializer>(init))
		{
		}
	};
#endif

	template <class Input>
	auto erase_unique(Input &&input)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
	    -> Si::unique_observable<typename std::decay<Input>::type::element_type>
#endif
	{
		typedef typename std::decay<Input>::type clean_input;
		typedef typename clean_input::element_type element_type;
		return Si::unique_observable<element_type>(
		    Si::make_unique<Si::virtualized_observable<clean_input, ptr_observer<observer<element_type>>>>(
		        std::forward<Input>(input)));
	}
}

#endif
