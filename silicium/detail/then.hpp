#ifndef SILICIUM_THEN_HPP
#define SILICIUM_THEN_HPP

#include <silicium/config.hpp>
#include <utility>
#include <type_traits>

#define SILICIUM_DETAIL_HAS_THEN SILICIUM_COMPILER_HAS_VARIADIC_TEMPLATES

namespace Si
{
#if SILICIUM_DETAIL_HAS_THEN
	namespace detail
	{
		template <class T>
		void default_construct(std::true_type)
		{
		}

		template <class T>
		T default_construct(std::false_type)
		{
			return T{};
		}

		template <class T>
		T default_construct()
		{
			return ::Si::detail::default_construct<T>(std::is_same<T, void>());
		}

		namespace detail
		{
			template <class Error>
			struct then_impl
			{
				Error operator()() const
				{
					return Error();
				}

				template <class First, class... Tail>
				Error operator()(First &&first, Tail &&... tail) const
				{
					auto error = std::forward<First>(first)();
					if (error)
					{
						return error;
					}
					return (*this)(std::forward<Tail>(tail)...);
				}
			};

			template <>
			struct then_impl<void>
			{
				void operator()() const
				{
				}

				template <class First, class... Tail>
				void operator()(First &&first, Tail &&... tail) const
				{
					std::forward<First>(first)();
					return (*this)(std::forward<Tail>(tail)...);
				}
			};
		}

		template <class First, class... Sequence>
		auto then(First &&first, Sequence &&... actions)
		    -> decltype(std::forward<First>(first)())
		{
			typedef decltype(std::forward<First>(first)()) result_type;
			return detail::then_impl<result_type>()(
			    std::forward<First>(first), std::forward<Sequence>(actions)...);
		}
	}
#endif
}

#endif
