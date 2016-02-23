#ifndef SILICIUM_REACTIVE_EXCHANGE_HPP
#define SILICIUM_REACTIVE_EXCHANGE_HPP

#include <silicium/move_if_noexcept.hpp>
#include <utility>
#include <boost/config.hpp>
#include <boost/concept_check.hpp>

namespace Si
{
	namespace detail
	{
		template <class T, class U>
		T exchange_impl(T &dest, U &&source, std::true_type)
		{
			auto old =
#if SILICIUM_HAS_MOVE_IF_NOEXCEPT
			    Si::move_if_noexcept
#else
			    // This is not exception safe, but at least we do not break
			    // exchange for unique_ptr on ancient compilers.
			    std::move
#endif
			    (dest);
#if SILICIUM_VC2012
			// silence a wrong "unreferenced parameter" warning
			boost::ignore_unused_variable_warning(source);
#endif
			dest = std::forward<U>(source);
			return old;
		}

		template <class T, class U>
		T exchange_impl(T &dest, U &&source, std::false_type)
		{
			auto old = dest;
			dest = std::forward<U>(source);
			return old;
		}
	}

	template <class T, class U>
	T exchange(T &dest, U &&source)
	{
		return detail::exchange_impl(
		    dest, std::forward<U>(source),
		    std::integral_constant<bool,
#if SILICIUM_COMPILER_HAS_WORKING_NOEXCEPT
		                           BOOST_NOEXCEPT_EXPR(
		                               dest = std::forward<U>(source))
#else
		                           true
#endif
		                           >());
	}
}

#endif
