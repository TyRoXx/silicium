#ifndef VENTURA_PATH_HPP
#define VENTURA_PATH_HPP

#include <ventura/path_char.hpp>
#include <boost/functional/hash.hpp>
#include <silicium/noexcept_string.hpp>
#include <boost/filesystem/path.hpp>

namespace ventura
{
	struct path
	{
		typedef native_path_char char_type;
		typedef
#ifdef _WIN32
			boost::filesystem::path
#else
			Si::noexcept_string
#endif
			underlying_type;

		path() BOOST_NOEXCEPT
		{
		}

		explicit path(Si::noexcept_string const &value)
			: m_value(value)
		{
		}

		explicit path(boost::filesystem::path const &value)
#ifdef _WIN32
		    : m_value(value)
#else
			: m_value(value.c_str())
#endif
		{
		}

		explicit path(char_type const *c_str)
			: m_value(c_str)
		{
		}

		template <std::size_t N>
		explicit path(char_type const (&c_str_literal)[N])
			: m_value(c_str_literal)
		{
		}

#ifdef _WIN32
		explicit path(char const *c_str)
			: m_value(c_str)
		{
		}

		template <std::size_t N>
		explicit path(char const (&c_str_literal)[N])
			: m_value(c_str_literal)
		{
		}
#endif

		template <class Iterator>
		path(Iterator begin, Iterator end)
		    : m_value(begin, end)
		{
		}

		path(path &&other) BOOST_NOEXCEPT
#ifndef _WIN32
		    : m_value(std::move(other.m_value))
#endif
		{
#ifdef _WIN32
			m_value.swap(other.m_value);
#endif
		}

		path(path const &other)
			: m_value(other.m_value)
		{
		}

		path &operator = (path &&other) BOOST_NOEXCEPT
		{
#ifdef _WIN32
			m_value.swap(other.m_value);
#else
			m_value = std::move(other.m_value);
#endif
			return *this;
		}

		path &operator = (path const &other)
		{
			m_value = other.m_value;
			return *this;
		}

		void swap(path &other) BOOST_NOEXCEPT
		{
			m_value.swap(other.m_value);
		}

		boost::filesystem::path
#ifdef _WIN32
		const &
#endif
		to_boost_path() const
		{
			return m_value
#ifndef _WIN32
			        .c_str()
#endif
			;
		}

#ifdef _WIN32
		boost::filesystem::path const &
#else
		Si::noexcept_string const &
#endif
		underlying() const BOOST_NOEXCEPT
		{
			return m_value;
		}

		char_type const *c_str() const BOOST_NOEXCEPT
		{
			return m_value.c_str();
		}

		void append(path const &right)
		{
			m_value += right.m_value;
		}

	private:

		underlying_type m_value;
	};

#if SILICIUM_HAS_IS_HANDLE
	BOOST_STATIC_ASSERT(Si::is_handle<path>::value);
#endif

	inline std::ostream &operator << (std::ostream &out, path const &p)
	{
		return out << p.underlying();
	}

	template <class ComparableToPath>
	inline bool operator == (path const &left, ComparableToPath const &right)
	{
		return left.underlying() == right;
	}

	template <class ComparableToPath>
	inline bool operator == (ComparableToPath const &left, path const &right)
	{
		return left == right.underlying();
	}

	inline bool operator == (path const &left, boost::filesystem::path const &right)
	{
		return right == left.c_str();
	}

	inline bool operator == (boost::filesystem::path const &left, path const &right)
	{
		return left == right.c_str();
	}

	inline bool operator == (path const &left, path const &right)
	{
		return left.underlying() == right.underlying();
	}

	template <class ComparableToPath>
	inline bool operator != (path const &left, ComparableToPath const &right)
	{
		return !(left == right);
	}

	template <class ComparableToPath>
	inline bool operator != (ComparableToPath const &left, path const &right)
	{
		return !(left == right);
	}

	inline bool operator < (path const &left, path const &right)
	{
		return left.underlying() < right.underlying();
	}

	inline std::size_t hash_value(path const &value)
	{
		using boost::hash_value;
		return hash_value(value.underlying());
	}

	inline path leaf(path const &whole)
	{
		//TODO: do this efficiently
		return path(whole.to_boost_path().leaf());
	}

	inline path parent(path const &whole)
	{
		//TODO: do this efficiently
		return path(whole.to_boost_path().parent_path());
	}

	inline path operator / (path const &front, path const &back)
	{
		//TODO: do this efficiently
		return path(front.to_boost_path() / back.to_boost_path());
	}
}

namespace std
{
	template <>
	struct hash< ::ventura::path>
	{
		std::size_t operator()(ventura::path const &value) const
		{
			return hash_value(value);
		}
	};
}

#endif
