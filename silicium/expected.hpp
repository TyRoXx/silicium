#ifndef SILICIUM_EXPECTED_HPP
#define SILICIUM_EXPECTED_HPP

#include <silicium/explicit_operator_bool.hpp>
#include <silicium/variant.hpp>

#define SILICIUM_HAS_EXPECTED (SILICIUM_HAS_EXCEPTIONS && SILICIUM_HAS_VARIANT)

#if SILICIUM_HAS_EXPECTED
#include <boost/exception_ptr.hpp>
namespace Si
{
	template <class T, class ExceptionPtr = boost::exception_ptr>
	struct expected
	{
		typedef T value_type;
		typedef ExceptionPtr exception_ptr;

		expected()
		{
		}

		template <class ...Args>
		explicit expected(Args &&...args)
			: content(inplace<T>(), std::forward<Args>(args)...)
		{
		}

		explicit expected(exception_ptr exception)
			: content(std::move(exception))
		{
		}

		T &value() &
		{
			return visit<T &>(
				content,
				[](T &value) -> T &
				{
					return value;
				},
				[](exception_ptr const &exception) -> T &
				{
					rethrow_exception(exception);
				}
			);
		}

		T const &value() const &
		{
			return visit<T const &>(
				content,
				[](T const &value) -> T const &
				{
					return value;
				},
				[](exception_ptr const &exception) -> T &
				{
					rethrow_exception(exception);
				}
			);
		}

		T &&value() &&
		{
			return visit<T &&>(
				content,
				[](T &value) -> T &&
				{
					return std::move(value);
				},
				[](exception_ptr const &exception) -> T &&
				{
					rethrow_exception(exception);
				}
			);
		}

		template <class ...Args>
		void emplace(Args &&...args)
		{
			T * const existing = try_get_ptr<T>(content);
			if (existing)
			{
				*existing = T(std::forward<Args>(args)...);
			}
			else
			{
				content = T(std::forward<Args>(args)...);
			}
		}

		bool valid() const BOOST_NOEXCEPT
		{
			return try_get_ptr<T>(content) != nullptr;
		}

		bool operator !() const BOOST_NOEXCEPT
		{
			return !valid();
		}

		SILICIUM_EXPLICIT_OPERATOR_BOOL()

	private:

		//TODO: avoid the dependency on variant/variadic templates
		variant<T, exception_ptr> content;
	};
}
#endif

#endif
