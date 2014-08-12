#ifndef SILICIUM_REACTIVE_PTR_OBSERVABLE_HPP
#define SILICIUM_REACTIVE_PTR_OBSERVABLE_HPP

#include <silicium/observable.hpp>
#include <silicium/override.hpp>
#include <silicium/config.hpp>
#include <silicium/virtualize.hpp>
#include <boost/config.hpp>
#include <memory>
#include <cassert>

namespace Si
{
	template <class Element, class Ptr>
	struct ptr_observable : observable<Element>
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

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(content);
			return content->async_get_one(receiver);
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			assert(content);
			return content->cancel();
		}

	private:

		Ptr content;
	};

	template <class Element>
	using unique_observable = ptr_observable<Element, std::unique_ptr<observable<Element>>>;

	template <class Element, class Content>
	auto box(Content &&content) -> ptr_observable<Element, std::unique_ptr<observable<Element>>>
	{
		return ptr_observable<Element, std::unique_ptr<observable<Element>>>(std::unique_ptr<observable<Element>>(new typename std::decay<Content>::type(std::forward<Content>(content))));
	}

	template <class Element>
	using shared_observable = ptr_observable<Element, std::shared_ptr<observable<Element>>>;

	template <class Element, class Content>
	auto wrap(Content &&content) -> ptr_observable<Element, std::shared_ptr<typename std::decay<Content>::type>>
	{
		return ptr_observable<Element, std::shared_ptr<typename std::decay<Content>::type>>(std::make_shared<typename std::decay<Content>::type>(std::forward<Content>(content)));
	}

	template <class Observable, class ...Args>
	auto make_wrapped(Args &&...args) -> ptr_observable<typename Observable::element_type, std::shared_ptr<Observable>>
	{
		typedef typename Observable::element_type element_type;
		typedef std::shared_ptr<Observable> ptr_type;
		return ptr_observable<element_type, ptr_type>(std::make_shared<Observable>(std::forward<Args>(args)...));
	}

	template <class Input>
	auto erase_unique(Input &&input)
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
	auto erase_shared(Input &&input)
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
