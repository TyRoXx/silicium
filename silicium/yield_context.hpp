#ifndef SILICIUM_YIELD_CONTEXT_HPP
#define SILICIUM_YIELD_CONTEXT_HPP

#include <silicium/transform.hpp>
#include <silicium/virtualized_observable.hpp>
#include <silicium/ref.hpp>
#include <boost/optional.hpp>

namespace Si
{
	namespace detail
	{
		template <class Element>
		struct yield_context_impl
		{
			virtual ~yield_context_impl()
			{
			}

			virtual void push_result(Element result) = 0;
			virtual void get_one(observable<nothing> &target) = 0;
		};
	}

	template <class Element = nothing>
	struct yield_context
	{
		explicit yield_context(detail::yield_context_impl<Element> &impl)
			: impl(&impl)
		{
		}

		void operator()(Element result)
		{
			return impl->push_result(std::move(result));
		}

		template <class Observable, class Gotten = typename Observable::element_type>
		boost::optional<Gotten> get_one(Observable &from)
		{
			boost::optional<Gotten> result;
			auto tf = Si::virtualize_observable(Si::transform(Si::ref(from), [&result](Gotten element)
			{
				assert(!result);
				result = std::move(element);
				return nothing{};
			}));
			impl->get_one(tf);
			return result;
		}

	private:

		detail::yield_context_impl<Element> *impl = nullptr;
	};
}

#endif
