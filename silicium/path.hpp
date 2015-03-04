#ifndef SILICIUM_PATH_HPP
#define SILICIUM_PATH_HPP

#include <silicium/noexcept_string.hpp>
#include <boost/filesystem/path.hpp>

namespace Si
{
	typedef
#ifdef _WIN32
		wchar_t
#else
		char
#endif
		native_path_char;

	struct path
	{
		typedef native_path_char char_type;

		path() BOOST_NOEXCEPT
		{
		}

		explicit path(noexcept_string const &value)
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

		template <class Iterator>
		path(Iterator begin, Iterator end)
		    : m_value(begin, end)
		{
		}

		path(path &&other) BOOST_NOEXCEPT
		    : m_value(std::move(other.m_value))
		{
		}

		path(path const &other)
			: m_value(other.m_value)
		{
		}

		path &operator = (path &&other) BOOST_NOEXCEPT
		{
			m_value = std::move(other.m_value);
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
		noexcept_string const &
#endif
		underlying() const BOOST_NOEXCEPT
		{
			return m_value;
		}

		char_type const *c_str() const BOOST_NOEXCEPT
		{
			return m_value.c_str();
		}

	private:

#ifdef _WIN32
		boost::filesystem::path m_value;
#else
		noexcept_string m_value;
#endif
	};

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
		return right.compare(left.c_str()) == 0;
	}

	inline bool operator == (boost::filesystem::path const &left, path const &right)
	{
		return left.compare(right.c_str()) == 0;
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
}

namespace std
{
	template <>
	struct hash< ::Si::path>
	{
		std::size_t operator()(Si::path const &value) const
		{
			return hash_value(value);
		}
	};
}

#endif
