#ifndef SILICIUM_ERROR_OR_HPP
#define SILICIUM_ERROR_OR_HPP

#include <silicium/optional.hpp>
#include <boost/system/system_error.hpp>
#include <boost/throw_exception.hpp>
#include <boost/functional/hash.hpp>
#include <system_error>

namespace Si
{
	inline SILICIUM_NORETURN void throw_error(boost::system::error_code error)
	{
		boost::throw_exception(boost::system::system_error(error));
	}

	inline SILICIUM_NORETURN void throw_error(std::error_code error)
	{
		boost::throw_exception(std::system_error(error));
	}

	namespace detail
	{
		template <class To, class From>
		From &&convert_if_necessary(From &&from, std::true_type)
		{
			return std::forward<From>(from);
		}

		template <class To, class From>
		To convert_if_necessary(From &&from, std::false_type)
		{
			return To(std::forward<From>(from));
		}

		template <class ErrorCode>
		struct category
		{
			typedef typename std::decay<decltype(std::declval<ErrorCode>().category())>::type type;
		};
	}

	template <class Value, class Error = boost::system::error_code>
	struct error_or
	{
		error_or() BOOST_NOEXCEPT
			: code(0)
#if SILICIUM_COMPILER_HAS_CXX11_UNION
			, value(Value()) //initialize so that reading the value does not have undefined behaviour
#endif
		{
#if !SILICIUM_COMPILER_HAS_CXX11_UNION
			new (value_ptr()) Value();
#endif
		}

		template <class ConvertibleToValue, class = typename std::enable_if<std::is_convertible<ConvertibleToValue, Value>::value, void>::type>
		error_or(ConvertibleToValue &&value) BOOST_NOEXCEPT
			: code(0)
#if SILICIUM_COMPILER_HAS_CXX11_UNION
			, value(std::forward<ConvertibleToValue>(value))
#endif
		{
#if !SILICIUM_COMPILER_HAS_CXX11_UNION
			new (value_ptr()) Value(std::forward<ConvertibleToValue>(value));
#endif
		}

		error_or(Value &&value) BOOST_NOEXCEPT
			: code(0)
#if SILICIUM_COMPILER_HAS_CXX11_UNION
			, value_(std::move(value))
#endif
		{
#if !SILICIUM_COMPILER_HAS_CXX11_UNION
			new (value_ptr()) Value(std::move(value));
#endif
		}

		error_or(Value const &value)
			: code(0)
#if SILICIUM_COMPILER_HAS_CXX11_UNION
			, value_(value)
#endif
		{
#if !SILICIUM_COMPILER_HAS_CXX11_UNION
			new (value_ptr()) Value(value);
#endif
		}

		error_or(Error error) BOOST_NOEXCEPT
			: code(error.value())
			, category(&error.category())
		{
		}

		error_or(error_or const &other)
			: code(other.code)
		{
			if (other.is_error())
			{
				category = other.category;
			}
			else
			{
				new (value_ptr()) Value(*other.value_ptr());
			}
		}

		error_or(error_or &&other) BOOST_NOEXCEPT
			: code(other.code)
		{
			if (other.is_error())
			{
				category = other.category;
			}
			else
			{
				new (value_ptr()) Value(std::move(*other.value_ptr()));
			}
		}

		error_or &operator = (error_or &&other) BOOST_NOEXCEPT
		{
			if (is_error())
			{
				code = other.code;
				if (other.is_error())
				{
					category = other.category;
				}
				else
				{
					new (&value) Value(std::move(other.value));
				}
			}
			else
			{
				if (other.is_error())
				{
					value.~Value();
					code = other.code;
					category = other.category;
				}
				else
				{
					value = std::move(other.value);
				}
			}
			return *this;
		}

		error_or &operator = (error_or const &other)
		{
			error_or copy(other);
			*this = std::move(copy);
			return *this;
		}

		error_or &operator = (Value &&other) BOOST_NOEXCEPT
		{
			if (is_error())
			{
				code = 0;
				new (&value) Value(std::move(other));
			}
			else
			{
				value = std::move(other);
			}
			return *this;
		}

		error_or &operator = (Value const &other)
		{
			if (is_error())
			{
				code = 0;
				new (value_ptr()) Value(other);
			}
			else
			{
				*value_ptr() = other;
			}
			return *this;
		}

		~error_or() BOOST_NOEXCEPT
		{
			if (!is_error())
			{
				value_ptr()->~Value();
			}
		}

		bool is_error() const BOOST_NOEXCEPT
		{
			return code != 0;
		}

		Error error() const BOOST_NOEXCEPT
		{
			assert(is_error());
			return Error(code, *category);
		}

		Value &get()
#if SILICIUM_COMPILER_HAS_RVALUE_THIS_QUALIFIER
			&
#endif
		{
			if (is_error())
			{
				throw_error(error());
			}
			return *value_ptr();
		}

#if SILICIUM_COMPILER_HAS_RVALUE_THIS_QUALIFIER
		Value &&get() &&
		{
			if (is_error())
			{
				throw_error(error());
			}
			return std::move(value);
		}
#endif

		Value const &get() const
#if SILICIUM_COMPILER_HAS_RVALUE_THIS_QUALIFIER
			&
#endif
		{
			if (is_error())
			{
				throw_error(error());
			}
			return *value_ptr();
		}

		Value *get_ptr() BOOST_NOEXCEPT
		{
			if (is_error())
			{
				return nullptr;
			}
			return value_ptr();
		}

		Value const *get_ptr() const BOOST_NOEXCEPT
		{
			if (is_error())
			{
				return nullptr;
			}
			return value_ptr();
		}

		Value *operator -> ()
		{
			return &get();
		}

		Value const *operator -> () const
		{
			return &get();
		}

		boost::optional<Value> get_optional()
#if SILICIUM_COMPILER_HAS_RVALUE_THIS_QUALIFIER
			&&
#endif
		{
			auto *value = get_ptr();
			if (!value)
			{
				return boost::none;
			}
			return std::move(*value);
		}

		template <class ComparableToValue>
		bool equals(error_or<ComparableToValue, Error> const &other) const
		{
			if (is_error() != other.is_error())
			{
				return false;
			}
			if (is_error())
			{
				return error() == other.error();
			}
			return value == other.get();
		}

		template <class ConvertibleToValue, class = typename std::enable_if<std::is_convertible<ConvertibleToValue, Value>::value, void>::type>
		bool equals(ConvertibleToValue const &right) const
		{
			if (is_error())
			{
				return false;
			}
			return get() == detail::convert_if_necessary<Value>(right, std::is_same<typename std::decay<Value>::type, typename std::decay<ConvertibleToValue>::type>());
		}

		bool equals(Error const &right) const
		{
			if (!is_error())
			{
				return false;
			}
			return error() == right;
		}

	private:

		typedef typename detail::category<Error>::type category_type;

		int code;
		union
		{
#if SILICIUM_COMPILER_HAS_CXX11_UNION
			Value value_;
#else
			std::array<char, sizeof(Value)> value_;
#endif
			category_type const *category;
		};

		Value *value_ptr()
		{
#if SILICIUM_COMPILER_HAS_CXX11_UNION
			return &value_;
#else
			return reinterpret_cast<Value *>(value_.data());
#endif
		}

		Value const *value_ptr() const
		{
#if SILICIUM_COMPILER_HAS_CXX11_UNION
			return &value_;
#else
			return reinterpret_cast<Value const *>(value_.data());
#endif
		}
	};

	template <class T>
	struct is_error_or : std::false_type
	{
	};

	template <class Value, class Error>
	struct is_error_or<error_or<Value, Error>> : std::true_type
	{
	};

	template <class Value, class Error, class Anything>
	bool operator == (error_or<Value, Error> const &left, Anything const &right)
	{
		return left.equals(right);
	}

	template <class Anything, class Value, class Error, class = typename std::enable_if<!is_error_or<Anything>::value, void>::type>
	bool operator == (Anything const &left, error_or<Value, Error> const &right)
	{
		return right.equals(left);
	}

	template <class Anything, class Value, class Error>
	bool operator != (Anything const &left, error_or<Value, Error> const &right)
	{
		return !(left == right);
	}

	template <class Anything, class Value, class Error>
	bool operator != (error_or<Value, Error> const &left, Anything const &right)
	{
		return !(left == right);
	}

	template <class Value, class Error>
	bool operator < (error_or<Value, Error> const &left, error_or<Value, Error> const &right)
	{
		if (left.is_error())
		{
			if (right.is_error())
			{
				return left.error() < right.error();
			}
			else
			{
				return true;
			}
		}
		else
		{
			if (right.is_error())
			{
				return false;
			}
			else
			{
				return left.get() < right.get();
			}
		}
	}

	//TODO: overload all operators

	template <class Value, class Error>
	std::size_t hash_value(error_or<Value, Error> const &value)
	{
		if (value.is_error())
		{
			return boost::hash<Error>()(value.error());
		}
		else
		{
			return boost::hash<Value>()(value.get());
		}
	}

	template <class Value, class Error>
	std::ostream &operator << (std::ostream &out, error_or<Value, Error> const &value)
	{
		if (value.is_error())
		{
			return out << value.error();
		}
		return out << value.get();
	}

	template <class ErrorOr, class OnValue, class CleanErrorOr = typename std::decay<ErrorOr>::type, class = typename std::enable_if<is_error_or<CleanErrorOr>::value, void>::type>
	auto map(ErrorOr &&maybe, OnValue &&on_value)
		-> error_or<decltype(std::forward<OnValue>(on_value)(std::forward<ErrorOr>(maybe).get()))>
	{
		if (maybe.is_error())
		{
			return maybe.error();
		}
		return std::forward<OnValue>(on_value)(std::forward<ErrorOr>(maybe).get());
	}
}

namespace std
{
	template <class Value, class Error>
	struct hash<Si::error_or<Value, Error>>
	{
		std::size_t operator()(Si::error_or<Value, Error> const &value) const
		{
			return hash_value(value);
		}
	};
}

#endif
