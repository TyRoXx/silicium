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

		expected(T &&value)
			: content(std::move(value))
		{
		}

		expected(T const &value)
			: content(std::move(value))
		{
		}

		template <class ...Args>
		explicit expected(Args &&...args)
			: content(inplace<T>(), std::forward<Args>(args)...)
		{
		}

		expected(exception_ptr exception)
			: content(std::move(exception))
		{
		}

		expected(expected &&other)
			: content(std::move(other.content))
		{
		}

		expected(expected const &other)
			: content(other.content)
		{
		}

		expected &operator = (T &&value)
		{
			T * const existing = try_get_ptr<T>(content);
			if (existing)
			{
				*existing = std::move(value);
			}
			else
			{
				content = std::move(value);
			}
			return *this;
		}

		expected &operator = (T const &value)
		{
			T * const existing = try_get_ptr<T>(content);
			if (existing)
			{
				*existing = value;
			}
			else
			{
				content = value;
			}
			return *this;
		}

		expected &operator = (expected &&other)
		{
			content = std::move(other.content);
			return *this;
		}

		expected &operator = (expected const &other)
		{
			content = other.content;
			return *this;
		}

		T &value()
#if SILICIUM_COMPILER_HAS_RVALUE_THIS_QUALIFIER
			&
#endif
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

		T const &value() const
#if SILICIUM_COMPILER_HAS_RVALUE_THIS_QUALIFIER
			&
#endif
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

#if SILICIUM_COMPILER_HAS_RVALUE_THIS_QUALIFIER
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
#endif

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
		
	private:

		//TODO: avoid the dependency on variant/variadic templates
		variant<T, exception_ptr> content;
	};
}
#endif

#endif
