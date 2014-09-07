#ifndef SILICIUM_SOURCE_OBSERVABLE_HPP
#define SILICIUM_SOURCE_OBSERVABLE_HPP

#include <silicium/observer.hpp>
#include <utility>

namespace Si
{
	template <class Element, class Source>
	struct source_observable
	{
		using element_type = Element;

		source_observable()
		{
		}

		explicit source_observable(Source source)
			: source(std::move(source))
		{
		}

		void async_get_one(observer<Element> &receiver)
		{
			boost::optional<Element> value = Si::get(source);
			if (value)
			{
				receiver.got_element(std::move(*value));
			}
			else
			{
				receiver.ended();
			}
		}

	private:

		Source source;
	};

	template <class Element, class Source>
	auto make_source_observable(Source &&source)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> source_observable<Element, typename std::decay<Source>::type>
#endif
	{
		return source_observable<Element, typename std::decay<Source>::type>(std::forward<Source>(source));
	}
}

#endif
