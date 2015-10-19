#ifndef VENTURA_RELATIVE_PATH_HPP
#define VENTURA_RELATIVE_PATH_HPP

#include <ventura/path_segment.hpp>
#include <silicium/noexcept_string.hpp>
#include <boost/filesystem/path.hpp>

namespace ventura
{
	struct relative_path
	{
		typedef native_path_char char_type;

		relative_path() BOOST_NOEXCEPT
		{
		}

		explicit relative_path(Si::noexcept_string const &value)
		    : m_value(value)
		{
		}

		explicit relative_path(path_segment const &segment)
			: m_value(segment.underlying())
		{
		}

		explicit relative_path(boost::filesystem::path const &value)
#ifdef _WIN32
		    : m_value(value)
#else
			: m_value(value.c_str())
#endif
		{
		}

		explicit relative_path(char_type const *c_str)
			: m_value(c_str)
		{
		}

		template <std::size_t N>
		explicit relative_path(char_type const (&c_str_literal)[N])
			: m_value(c_str_literal)
		{
		}

#ifdef _WIN32
		explicit relative_path(char const *c_str)
			: m_value(c_str)
		{
		}

		template <std::size_t N>
		explicit relative_path(char const (&c_str_literal)[N])
			: m_value(c_str_literal)
		{
		}
#endif

		template <class Iterator>
		relative_path(Iterator begin, Iterator end)
		    : m_value(begin, end)
		{
		}

		relative_path(relative_path &&other) BOOST_NOEXCEPT
		    : m_value(std::move(other.m_value))
		{
		}

		relative_path(relative_path const &other)
			: m_value(other.m_value)
		{
		}

		relative_path &operator = (relative_path &&other) BOOST_NOEXCEPT
		{
			m_value = std::move(other.m_value);
			return *this;
		}

		relative_path &operator = (relative_path const &other)
		{
			m_value = other.m_value;
			return *this;
		}

		void swap(relative_path &other) BOOST_NOEXCEPT
		{
			m_value.swap(other.m_value);
		}

		SILICIUM_USE_RESULT
		boost::filesystem::path
#ifdef _WIN32
		const &
#endif
		to_boost_path() const
		{
			return m_value.to_boost_path();
		}

		SILICIUM_USE_RESULT
#ifdef _WIN32
		boost::filesystem::path const &
#else
		Si::noexcept_string const &
#endif
		underlying() const BOOST_NOEXCEPT
		{
			return m_value.underlying();
		}

		SILICIUM_USE_RESULT
		char_type const *c_str() const BOOST_NOEXCEPT
		{
			return m_value.c_str();
		}

		SILICIUM_USE_RESULT
		bool empty() const BOOST_NOEXCEPT
		{
			return underlying().empty();
		}

	private:

		path m_value;
	};

#if SILICIUM_HAS_IS_HANDLE
	BOOST_STATIC_ASSERT(Si::is_handle<relative_path>::value);
#endif

	inline std::ostream &operator << (std::ostream &out, relative_path const &p)
	{
		return out << p.underlying();
	}

	template <class ComparableToPath>
	SILICIUM_USE_RESULT
	inline bool operator == (relative_path const &left, ComparableToPath const &right)
	{
		return left.underlying() == right;
	}

	template <class ComparableToPath>
	SILICIUM_USE_RESULT
	inline bool operator == (ComparableToPath const &left, relative_path const &right)
	{
		return left == right.underlying();
	}

	SILICIUM_USE_RESULT
	inline bool operator == (relative_path const &left, boost::filesystem::path const &right)
	{
		return right == left.c_str();
	}

	SILICIUM_USE_RESULT
	inline bool operator == (boost::filesystem::path const &left, relative_path const &right)
	{
		return left == right.c_str();
	}

	SILICIUM_USE_RESULT
	inline bool operator == (relative_path const &left, relative_path const &right)
	{
		return left.underlying() == right.underlying();
	}

	template <class ComparableToPath>
	SILICIUM_USE_RESULT
	inline bool operator != (relative_path const &left, ComparableToPath const &right)
	{
		return !(left == right);
	}

	template <class ComparableToPath>
	SILICIUM_USE_RESULT
	inline bool operator != (ComparableToPath const &left, relative_path const &right)
	{
		return !(left == right);
	}

	SILICIUM_USE_RESULT
	inline bool operator < (relative_path const &left, relative_path const &right)
	{
		return left.underlying() < right.underlying();
	}

	SILICIUM_USE_RESULT
	inline std::size_t hash_value(relative_path const &value)
	{
		using boost::hash_value;
		return hash_value(value.underlying());
	}

	SILICIUM_USE_RESULT
	inline relative_path leaf(relative_path const &whole)
	{
		//TODO: do this efficiently
		return relative_path(whole.to_boost_path().leaf());
	}

	SILICIUM_USE_RESULT
	inline relative_path parent(relative_path const &whole)
	{
		//TODO: do this efficiently
		return relative_path(whole.to_boost_path().parent_path());
	}

	SILICIUM_USE_RESULT
	inline relative_path operator / (relative_path const &front, relative_path const &back)
	{
		//TODO: do this efficiently
		return relative_path(front.to_boost_path() / back.to_boost_path());
	}

	inline relative_path operator / (path_segment const &front, path_segment const &back)
	{
		//TODO: do this efficiently
		relative_path result(front.to_boost_path() / back.to_boost_path());
		return result;
	}

	inline relative_path operator / (relative_path const &front, path_segment const &back)
	{
		return front / relative_path(back.to_boost_path());
	}
}

namespace std
{
	template <>
	struct hash< ::ventura::relative_path>
	{
		SILICIUM_USE_RESULT
		std::size_t operator()(ventura::relative_path const &value) const
		{
			return hash_value(value);
		}
	};
}

#endif
