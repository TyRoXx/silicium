#ifndef SILICIUM_OPTIONAL_HPP
#define SILICIUM_OPTIONAL_HPP

#include <silicium/fast_variant.hpp>
#include <ostream>

namespace Si
{
	using none_t = nothing;

	static none_t const none;

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

		optional(T &&value)
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

		optional &operator = (T &&value)
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

#ifdef _MSC_VER
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
	bool operator != (optional<T> const &left, optional<T> const &right)
	{
		return !(left == right);
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

	BOOST_STATIC_ASSERT(sizeof(optional<char>) == sizeof(unsigned));
	BOOST_STATIC_ASSERT(sizeof(optional<short>) == sizeof(unsigned));
	BOOST_STATIC_ASSERT(sizeof(optional<unsigned>) == (2 * sizeof(unsigned)));
	BOOST_STATIC_ASSERT(sizeof(optional<char *>) == (sizeof(unsigned) + sizeof(char *)));
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
