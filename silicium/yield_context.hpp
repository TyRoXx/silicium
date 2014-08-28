#ifndef SILICIUM_YIELD_CONTEXT_HPP
#define SILICIUM_YIELD_CONTEXT_HPP

#include <silicium/fast_variant.hpp>
#include <silicium/observable.hpp>

namespace Si
{
	namespace detail
	{
		template <class Element>
		struct result
		{
			Element value;

			result()
			{
			}

			explicit result(Element value)
				: value(std::move(value))
			{
			}

#ifdef _MSC_VER
			result(result &&other)
				: value(std::move(other.value))
			{
			}

			result &operator = (result &&other)
			{
				value = std::move(other.value);
				return *this;
			}

			result(result const &other)
				: value(other.value)
			{
			}

			result &operator = (result const &other)
			{
				value = other.value;
				return *this;
			}
#endif
		};

		struct yield
		{
			Si::observable<nothing> *target;
		};

		template <class Element>
		struct make_command
		{
			typedef Si::fast_variant<result<Element>, yield> type;
		};

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
			auto tf = Si::virtualize(Si::transform(Si::ref(from), [&result](Gotten element)
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
