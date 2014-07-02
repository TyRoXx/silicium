#ifndef SILICIUM_REACTIVE_PTR_OBSERVABLE_HPP
#define SILICIUM_REACTIVE_PTR_OBSERVABLE_HPP

#include <reactive/observable.hpp>
#include <silicium/override.hpp>
#include <memory>

namespace rx
{
	template <class Element, class Ptr>
	struct ptr_observable : observable<Element>
	{
		typedef Element element_type;

		explicit ptr_observable(Ptr content)
			: content(std::move(content))
		{
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

	template <class Element, class Content>
	auto box(Content &&content)
	{
		return ptr_observable<Element, std::unique_ptr<observable<Element>>>(std::unique_ptr<observable<Element>>(new typename std::decay<Content>::type(std::forward<Content>(content))));
	}

	template <class Element, class Content>
	auto wrap(Content &&content)
	{
		return ptr_observable<Element, std::shared_ptr<observable<Element>>>(std::make_shared<typename std::decay<Content>::type>(std::forward<Content>(content)));
	}

	template <class Element>
	auto ref(rx::observable<Element> &identity)
	{
		return rx::ptr_observable<Element, rx::observable<Element> *>(&identity);
	}

	template <class Observable, class ...Args>
	auto make_wrapped(Args &&...args)
	{
		typedef typename Observable::element_type element_type;
		typedef std::shared_ptr<Observable> ptr_type;
		return ptr_observable<element_type, ptr_type>(std::make_shared<Observable>(std::forward<Args>(args)...));
	}
}

#endif
