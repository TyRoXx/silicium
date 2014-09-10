#ifndef SILICIUM_REACTIVE_PTR_OBSERVABLE_HPP
#define SILICIUM_REACTIVE_PTR_OBSERVABLE_HPP

#include <silicium/observer.hpp>
#include <silicium/override.hpp>
#include <silicium/config.hpp>
#include <silicium/virtualized_observable.hpp>
#include <boost/config.hpp>
#include <memory>
#include <cassert>

namespace Si
{
	template <class Element, class Ptr>
	struct ptr_observable
	{
		typedef Element element_type;

		BOOST_DEFAULTED_FUNCTION(ptr_observable(), {})

		explicit ptr_observable(Ptr content)
			: content(std::move(content))
		{
		}

#ifdef _MSC_VER
		ptr_observable(ptr_observable &&other)
			: content(std::move(other.content))
		{
		}

		ptr_observable &operator = (ptr_observable &&other)
		{
			content = std::move(other.content);
			return *this;
		}

		ptr_observable(ptr_observable const &other)
			: content(other.content)
		{
		}

		ptr_observable &operator = (ptr_observable const &other)
		{
			content = other.content;
			return *this;
		}
#endif

		bool empty() const BOOST_NOEXCEPT
		{
			return !content;
		}

		void async_get_one(observer<element_type> &receiver)
		{
			assert(content);
			return content->async_get_one(receiver);
		}

	private:

		Ptr content;
	};

	template <class Element>
	using unique_observable = ptr_observable<Element, std::unique_ptr<observable<Element>>>;

	template <class Element>
	using shared_observable = ptr_observable<Element, std::shared_ptr<observable<Element>>>;

	template <class Input>
	auto erase_unique(Input &&input) -> Si::unique_observable<typename std::decay<Input>::type::element_type>
	{
		using clean_input = typename std::decay<Input>::type;
		using element_type = typename clean_input::element_type;
		return Si::unique_observable<element_type>(
					Si::make_unique<Si::virtualized_observable<clean_input>>(
						std::forward<Input>(input)
					)
				);
	}

	template <class Input>
	auto erase_shared(Input &&input) -> Si::shared_observable<typename std::decay<Input>::type::element_type>
	{
		using clean_input = typename std::decay<Input>::type;
		using element_type = typename clean_input::element_type;
		return Si::shared_observable<element_type>(
					std::make_shared<Si::virtualized_observable<clean_input>>(
						std::forward<Input>(input)
					)
				);
	}
}

#endif
