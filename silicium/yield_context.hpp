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
		struct basic_yield_context
		{
			virtual ~basic_yield_context()
			{
			}
			virtual void get_one(observable<nothing> &target) = 0;
		};

		template <class Element>
		struct push_context_impl : basic_yield_context
		{
			virtual void push_result(Element result) = 0;
		};
	}

	struct yield_context
	{
		explicit yield_context(detail::basic_yield_context &impl)
			: impl(&impl)
		{
		}

		template <class Observable, class Gotten = typename std::decay<Observable>::type::element_type>
		boost::optional<Gotten> get_one(Observable &&from)
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

	protected:

		detail::basic_yield_context *impl;
	};

	template <class Element = nothing>
	struct push_context : yield_context
	{
		explicit push_context(detail::push_context_impl<Element> &impl)
			: yield_context(impl)
		{
		}

		void operator()(Element result)
		{
			return static_cast<detail::push_context_impl<Element> *>(impl)->push_result(std::move(result));
		}
	};
}

#endif
