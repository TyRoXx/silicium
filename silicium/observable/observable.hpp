#ifndef SILICIUM_REACTIVE_OBSERVABLE_HPP
#define SILICIUM_REACTIVE_OBSERVABLE_HPP

#include <silicium/observable/observer.hpp>
#include <silicium/config.hpp>
#include <boost/concept_check.hpp>

namespace Si
{
	template <class Element, class Observer = ptr_observer<observer<Element>>>
	struct observable
	{
		typedef Element element_type;

		virtual ~observable()
		{
		}

		virtual void async_get_one(Observer receiver) = 0;
	};

	template <class X>
	struct Observable
	{
		typedef typename X::element_type element_type;

		BOOST_CONCEPT_USAGE(Observable)
		{
			X moved = std::move(observable);
			moved = std::move(observable);
			observable.async_get_one(observe_by_ref(observer_));
			X default_constructible;
			boost::ignore_unused_variable_warning(default_constructible);
		}

	private:

		struct test_observer : observer<element_type>
		{
			virtual void got_element(element_type) SILICIUM_OVERRIDE
			{
			}

			virtual void ended() SILICIUM_OVERRIDE
			{
			}
		};

		X observable;
		test_observer observer_;
		element_type element;
	};

	namespace detail
	{
		//example and test for the smallest possible Observable
		struct minimum_observable
		{
			typedef int element_type;

			template <class Observer>
			void async_get_one(Observer &&)
			{
			}
		};

		BOOST_CONCEPT_ASSERT((Observable<minimum_observable>));
	}
}

#endif
