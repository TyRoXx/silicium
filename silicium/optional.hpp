#ifndef SILICIUM_OPTIONAL_HPP
#define SILICIUM_OPTIONAL_HPP

#include <silicium/fast_variant.hpp>
#include <ostream>

namespace Si
{
	typedef nothing none_t;

	static none_t BOOST_CONSTEXPR_OR_CONST none;

	template <class T>
	struct optional
	{
		optional() BOOST_NOEXCEPT
		{
		}

		optional(none_t) BOOST_NOEXCEPT
		{
		}

		optional(optional &&other) BOOST_NOEXCEPT
			: content(std::move(other.content))
		{
		}

		optional(optional const &other)
			: content(other.content)
		{
		}

		optional(T &&value) BOOST_NOEXCEPT
			: content(std::move(value))
		{
		}

		optional(T const &value)
			: content(value)
		{
		}

		optional &operator = (optional &&other) BOOST_NOEXCEPT
		{
			content = std::move(other.content);
			return *this;
		}

		optional &operator = (optional const &other)
		{
			content = other.content;
			return *this;
		}

		optional &operator = (T const &value)
		{
			content = value;
			return *this;
		}

		optional &operator = (T &&value) BOOST_NOEXCEPT
		{
			content = std::move(value);
			return *this;
		}

		explicit operator bool() const BOOST_NOEXCEPT
		{
			return content.which() != 0;
		}

		bool operator !() const BOOST_NOEXCEPT
		{
			return content.which() == 0;
		}

#if !SILICIUM_COMPILER_HAS_RVALUE_THIS_QUALIFIER
		T &operator * () BOOST_NOEXCEPT
		{
			assert(*this);
			return *try_get_ptr<T>(content);
		}

		T const &operator * () const BOOST_NOEXCEPT
		{
			assert(*this);
			return *try_get_ptr<T>(content);
		}
#else
		T &operator * () & BOOST_NOEXCEPT
		{
			assert(*this);
			return *try_get_ptr<T>(content);
		}

		T &&operator * () && BOOST_NOEXCEPT
		{
			assert(*this);
			return std::move(*try_get_ptr<T>(content));
		}

		T const &operator * () const & BOOST_NOEXCEPT
		{
			assert(*this);
			return *try_get_ptr<T>(content);
		}
#endif

		T *operator -> () BOOST_NOEXCEPT
		{
			assert(*this);
			return try_get_ptr<T>(content);
		}

		T const *operator -> () const BOOST_NOEXCEPT
		{
			assert(*this);
			return try_get_ptr<T>(content);
		}

	private:

		fast_variant<none_t, T> content;
	};

	template <class T>
	bool operator == (optional<T> const &left, optional<T> const &right)
	{
		if (left && right)
		{
			return (*left == *right);
		}
		return !left == !right;
	}

	template <class T>
	bool operator == (optional<T> const &left, T const &right)
	{
		if (left)
		{
			return (*left == right);
		}
		return false;
	}

	template <class T>
	bool operator == (T const &left, optional<T> const &right)
	{
		return (right == left);
	}

	template <class T>
	bool operator == (none_t const &, optional<T> const &right)
	{
		return !right;
	}

	template <class T>
	bool operator == (optional<T> const &left, none_t const &)
	{
		return !left;
	}

	template <class T>
	bool operator != (optional<T> const &left, optional<T> const &right)
	{
		return !(left == right);
	}

	template <class T>
	Si::optional<typename std::decay<T>::type> make_optional(T &&value)
	{
		return Si::optional<typename std::decay<T>::type>(std::forward<T>(value));
	}

	inline std::ostream &operator << (std::ostream &out, none_t const &)
	{
		out << "none";
		return out;
	}

	template <class T>
	std::ostream &operator << (std::ostream &out, optional<T> const &value)
	{
		if (value)
		{
			out << *value;
		}
		else
		{
			out << "none";
		}
		return out;
	}

	BOOST_STATIC_ASSERT(sizeof(optional<boost::int8_t>) == sizeof(boost::uint32_t));
	BOOST_STATIC_ASSERT(sizeof(optional<boost::int16_t>) == sizeof(boost::uint32_t));
	BOOST_STATIC_ASSERT(sizeof(optional<boost::uint32_t>) == (2 * sizeof(boost::uint32_t)));
	BOOST_STATIC_ASSERT(sizeof(optional<char *>) == (sizeof(boost::uint32_t) + sizeof(char *)));
}

namespace std
{
	template <class T>
	struct hash<Si::optional<T>>
	{
		std::size_t operator()(Si::optional<T> const &v) const;
	};
}

#endif
