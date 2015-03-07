#ifndef SILICIUM_OPTIONAL_HPP
#define SILICIUM_OPTIONAL_HPP

#include <silicium/config.hpp>
#include <ostream>
#include <type_traits>
#include <boost/static_assert.hpp>

namespace Si
{
	typedef nothing none_t;

	static none_t BOOST_CONSTEXPR_OR_CONST none;

	struct some_t {};

	static some_t BOOST_CONSTEXPR_OR_CONST some;

	template <class T>
	struct optional
	{
		optional() BOOST_NOEXCEPT
			: m_is_set(false)
		{
		}

		optional(none_t) BOOST_NOEXCEPT
			: m_is_set(false)
		{
		}

		optional(optional &&other) BOOST_NOEXCEPT
			: m_is_set(other.m_is_set)
		{
			if (m_is_set)
			{
				new (data())T(std::move(*other));
			}
		}

		optional(optional const &other)
			: m_is_set(other.m_is_set)
		{
			if (m_is_set)
			{
				new (data())T(*other);
			}
		}

		optional(T &&value) BOOST_NOEXCEPT
			: m_is_set(true)
		{
			new (data()) T(std::move(value));
		}

		optional(T const &value)
			: m_is_set(true)
		{
			new (data()) T(value);
		}

		template <class ...Args>
		explicit optional(some_t, Args &&...args)
			: m_is_set(true)
		{
			new (data()) T(std::forward<Args>(args)...);
		}

		~optional() BOOST_NOEXCEPT
		{
			if (!m_is_set)
			{
				return;
			}
			data()->~T();
		}

		optional &operator = (optional &&other) BOOST_NOEXCEPT
		{
			if (m_is_set)
			{
				if (other.m_is_set)
				{
					*data() = std::move(*other);
				}
				else
				{
					data()->~T();
					m_is_set = false;
				}
			}
			else
			{
				if (other.m_is_set)
				{
					new (data()) T(std::move(*other));
					m_is_set = true;
				}
				else
				{
					//both are already empty
				}
			}
			return *this;
		}

		optional &operator = (optional const &other)
		{
			if (m_is_set)
			{
				if (other.m_is_set)
				{
					*data() = *other;
				}
				else
				{
					data()->~T();
					m_is_set = false;
				}
			}
			else
			{
				if (other.m_is_set)
				{
					new (data()) T(*other);
					m_is_set = true;
				}
				else
				{
					//both are already empty
				}
			}
			return *this;
		}

		optional &operator = (T const &value)
		{
			if (m_is_set)
			{
				*data() = value;
			}
			else
			{
				new (data()) T(value);
				m_is_set = true;
			}
			return *this;
		}

		optional &operator = (T &&value) BOOST_NOEXCEPT
		{
			if (m_is_set)
			{
				*data() = std::move(value);
			}
			else
			{
				new (data()) T(std::move(value));
				m_is_set = true;
			}
			return *this;
		}

		optional &operator = (none_t const &)BOOST_NOEXCEPT
		{
			if (m_is_set)
			{
				data()->~T();
				m_is_set = false;
			}
			return *this;
		}

		explicit operator bool() const BOOST_NOEXCEPT
		{
			return m_is_set;
		}

		bool operator !() const BOOST_NOEXCEPT
		{
			return !m_is_set;
		}

#if !SILICIUM_COMPILER_HAS_RVALUE_THIS_QUALIFIER
		T &operator * () BOOST_NOEXCEPT
		{
			assert(*this);
			return *data();
		}

		T const &operator * () const BOOST_NOEXCEPT
		{
			assert(*this);
			return *data();
		}
#else
		T &operator * () & BOOST_NOEXCEPT
		{
			assert(*this);
			return *data();
		}

		T &&operator * () && BOOST_NOEXCEPT
		{
			assert(*this);
			return std::move(*data());
		}

		T const &operator * () const & BOOST_NOEXCEPT
		{
			assert(*this);
			return *data();
		}
#endif

		T *operator -> () BOOST_NOEXCEPT
		{
			assert(*this);
			return data();
		}

		T const *operator -> () const BOOST_NOEXCEPT
		{
			assert(*this);
			return data();
		}

	private:

		enum
		{
			alignment =
#ifdef _MSC_VER
			__alignof(T)
#else
			alignof(T)
#endif
		};

		typename std::aligned_storage<sizeof(T), alignment>::type m_storage;
		bool m_is_set;

		T *data() BOOST_NOEXCEPT
		{
			return reinterpret_cast<T *>(&m_storage);
		}

		T const *data() const BOOST_NOEXCEPT
		{
			return reinterpret_cast<T const *>(&m_storage);
		}
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
	bool operator < (optional<T> const &left, optional<T> const &right)
	{
		if (left)
		{
			if (right)
			{
				return (*left < *right);
			}
			else
			{
				return false;
			}
		}
		else
		{
			if (right)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
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

	template <class T>
	std::size_t hash_value(optional<T> const &value)
	{
		if (value)
		{
			using boost::hash_value;
			return hash_value(*value);
		}
		return 0;
	}

	BOOST_STATIC_ASSERT(sizeof(optional<boost::int8_t>) == 2);
	BOOST_STATIC_ASSERT(sizeof(optional<boost::int16_t>) == 4);
	BOOST_STATIC_ASSERT(sizeof(optional<boost::uint32_t>) == (2 * sizeof(boost::uint32_t)));
	BOOST_STATIC_ASSERT(sizeof(optional<char *>) == (sizeof(boost::uint32_t) + sizeof(char *)));
}

namespace std
{
	template <class T>
	struct hash<Si::optional<T>>
	{
		std::size_t operator()(Si::optional<T> const &value) const
		{
			return hash_value(value);
		}
	};
}

#endif
