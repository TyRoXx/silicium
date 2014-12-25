#ifndef SILICIUM_FUNCTION_OBSERVER_HPP
#define SILICIUM_FUNCTION_OBSERVER_HPP

#include <silicium/observable/observer.hpp>
#include <silicium/detail/argument_of.hpp>
#include <silicium/detail/element_from_optional_like.hpp>
#include <silicium/optional.hpp>
#include <boost/optional.hpp>

namespace Si
{
	namespace detail
	{
		template <class Element>
		struct optional_maker
		{
			typename std::decay<Element>::type *value;

			template <class T>
			operator boost::optional<T>() const
			{
				return static_cast<T>(std::forward<Element>(*value));
			}

			template <class T>
			operator Si::optional<T>() const
			{
				return static_cast<T>(std::forward<Element>(*value));
			}
		};
	}

	template <class Function>
	struct function_observer
	{
		typedef typename detail::element_from_optional_like<
			typename std::decay<
				typename detail::argument_of<Function>::type
			>::type
		>::type element_type;

		explicit function_observer(Function function)
			: m_function(std::move(function))
		{
		}

		template <class Element>
		void got_element(Element &&element)
		{
			m_function(detail::optional_maker<Element>{&element});
		}

		void ended()
		{
			m_function(Si::none);
		}

	private:

		Function m_function;
	};

	BOOST_STATIC_ASSERT(std::is_same<int, function_observer<void (*)(boost::optional<int> const &)>::element_type>::value);

	template <class Function>
	auto make_function_observer(Function &&function)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> function_observer<typename std::decay<Function>::type>
#endif
	{
		return function_observer<typename std::decay<Function>::type>(std::forward<Function>(function));
	}
}

#endif
