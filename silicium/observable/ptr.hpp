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

		ptr_observable &operator=(ptr_observable &&other)
		{
			content = std::move(other.content);
			return *this;
		}

		ptr_observable(ptr_observable const &other)
		    : content(other.content)
		{
		}

		ptr_observable &operator=(ptr_observable const &other)
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
}

#endif
