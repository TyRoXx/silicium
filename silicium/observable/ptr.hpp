#ifndef SILICIUM_REACTIVE_PTR_OBSERVABLE_HPP
#define SILICIUM_REACTIVE_PTR_OBSERVABLE_HPP

#include <silicium/observable/observer.hpp>
#include <silicium/config.hpp>
#include <silicium/observable/virtualized.hpp>
#include <boost/config.hpp>
#include <memory>
#include <cassert>

namespace Si
{
	template <class Element, class Ptr>
	struct ptr_observable
	{
		typedef Element element_type;

		ptr_observable()
		{
		}

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

		template <class Observer>
		void async_get_one(Observer &&observer)
		{
			assert(content);
			return content->async_get_one(std::forward<Observer>(observer));
		}

	private:

		Ptr content;
	};

#if SILICIUM_COMPILER_HAS_USING
	template <class Element>
	using unique_observable = ptr_observable<Element, std::unique_ptr<observable<Element>>>;
#else
	template <class Element>
	struct unique_observable : ptr_observable<Element, std::unique_ptr<observable<Element>>>
	{
		typedef ptr_observable<Element, std::unique_ptr<observable<Element>>> base;

		template <class Initializer>
		unique_observable(Initializer &&init)
			: base(std::forward<Initializer>(init))
		{
		}
	};
#endif

#if SILICIUM_COMPILER_HAS_USING
	template <class Element>
	using shared_observable = ptr_observable<Element, std::shared_ptr<observable<Element>>>;
#else
	template <class Element>
	struct shared_observable : ptr_observable<Element, std::shared_ptr<observable<Element>>>
	{
		typedef ptr_observable<Element, std::shared_ptr<observable<Element>>> base;

		template <class Initializer>
		shared_observable(Initializer &&init)
			: base(std::forward<Initializer>(init))
		{
		}
	};
#endif

	template <class Input>
	auto erase_unique(Input &&input) -> Si::unique_observable<typename std::decay<Input>::type::element_type>
	{
		typedef typename std::decay<Input>::type clean_input;
		typedef typename clean_input::element_type element_type;
		return Si::unique_observable<element_type>(
					Si::make_unique<Si::virtualized_observable<clean_input>>(
						std::forward<Input>(input)
					)
				);
	}

	template <class Input>
	auto erase_shared(Input &&input) -> Si::shared_observable<typename std::decay<Input>::type::element_type>
	{
		typedef typename std::decay<Input>::type clean_input;
		typedef typename clean_input::element_type element_type;
		return Si::shared_observable<element_type>(
					std::make_shared<Si::virtualized_observable<clean_input>>(
						std::forward<Input>(input)
					)
				);
	}
}

#endif
