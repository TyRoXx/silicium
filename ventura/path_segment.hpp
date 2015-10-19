#ifndef VENTURA_PATH_SEGMENT_HPP
#define VENTURA_PATH_SEGMENT_HPP

#include <ventura/path.hpp>
#include <silicium/optional.hpp>
#include <boost/filesystem/path.hpp>

namespace ventura
{
	struct path_segment
	{
		typedef native_path_char char_type;

		path_segment() BOOST_NOEXCEPT
		{
		}

		path_segment(path_segment &&other) BOOST_NOEXCEPT
		    : m_value(std::move(other.m_value))
		{
		}

		path_segment(path_segment const &other)
			: m_value(other.m_value)
		{
		}

		path_segment &operator = (path_segment &&other) BOOST_NOEXCEPT
		{
			m_value = std::move(other.m_value);
			return *this;
		}

		path_segment &operator = (path_segment const &other)
		{
			m_value = other.m_value;
			return *this;
		}

		void swap(path_segment &other) BOOST_NOEXCEPT
		{
			m_value.swap(other.m_value);
		}

		boost::filesystem::path
#ifdef _WIN32
		const &
#endif
		to_boost_path() const
		{
			return m_value.to_boost_path();
		}

#ifdef _WIN32
		boost::filesystem::path const &
#else
		Si::noexcept_string const &
#endif
		underlying() const BOOST_NOEXCEPT
		{
			return m_value.underlying();
		}

		char_type const *c_str() const BOOST_NOEXCEPT
		{
			return m_value.c_str();
		}

		void append(path_segment const &right)
		{
			m_value.append(right.m_value);
		}

		static Si::optional<path_segment> create(boost::filesystem::path const &maybe_segment)
		{
			if (maybe_segment.parent_path().empty())
			{
				return path_segment(maybe_segment);
			}
			return Si::none;
		}

	private:

		path m_value;

		explicit path_segment(boost::filesystem::path const &value)
			: m_value(value)
		{
		}
	};

#if SILICIUM_HAS_IS_HANDLE
	BOOST_STATIC_ASSERT(Si::is_handle<path_segment>::value);
#endif

	inline std::ostream &operator << (std::ostream &out, path_segment const &p)
	{
		return out << p.underlying();
	}

	template <class ComparableToPath>
	inline bool operator == (path_segment const &left, ComparableToPath const &right)
	{
		return left.underlying() == right;
	}

	template <class ComparableToPath>
	inline bool operator == (ComparableToPath const &left, path_segment const &right)
	{
		return left == right.underlying();
	}

	inline bool operator == (path_segment const &left, boost::filesystem::path const &right)
	{
		return left.underlying().c_str() == right;
	}

	inline bool operator == (boost::filesystem::path const &left, path_segment const &right)
	{
		return left == right.underlying().c_str();
	}

	inline bool operator == (path_segment const &left, path_segment const &right)
	{
		return left.underlying() == right.underlying();
	}

	template <class ComparableToPath>
	inline bool operator != (path_segment const &left, ComparableToPath const &right)
	{
		return !(left == right);
	}

	template <class ComparableToPath>
	inline bool operator != (ComparableToPath const &left, path_segment const &right)
	{
		return !(left == right);
	}

	inline bool operator < (path_segment const &left, path_segment const &right)
	{
		return left.underlying() < right.underlying();
	}

	inline path_segment operator + (path_segment left, path_segment const &right)
	{
		left.append(right);
		return left;
	}

	inline std::size_t hash_value(path_segment const &value)
	{
		using boost::hash_value;
		return hash_value(value.underlying());
	}
}

namespace std
{
	template <>
	struct hash< ::ventura::path_segment>
	{
		std::size_t operator()(ventura::path_segment const &value) const
		{
			return hash_value(value);
		}
	};
}

#endif
