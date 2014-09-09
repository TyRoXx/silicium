#ifndef SILICIUM_REACTIVE_OBSERVABLE_HPP
#define SILICIUM_REACTIVE_OBSERVABLE_HPP

#include <silicium/observer.hpp>
#include <silicium/override.hpp>
#include <boost/concept_check.hpp>

namespace Si
{
	template <class Element>
	struct observable
	{
		typedef Element element_type;

		virtual ~observable()
		{
		}

		virtual void async_get_one(observer<element_type> &receiver) = 0;
	};

	template <class X>
	struct Observable
	{
		using element_type = typename X::element_type;

		BOOST_CONCEPT_USAGE(Observable)
		{
			X moved = std::move(observable);
			moved = std::move(observable);
			observable.async_get_one(observer);
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
		test_observer observer;
		element_type element;
	};

	namespace detail
	{
		//example and test for the smallest possible Observable
		struct minimum_observable
		{
			using element_type = int;
			void async_get_one(observer<element_type> &)
			{
			}
		};

		BOOST_CONCEPT_ASSERT((Observable<minimum_observable>));
	}
}

#endif
