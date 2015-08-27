#ifndef SILICIUM_ABSOLUTE_PATH_HPP
#define SILICIUM_ABSOLUTE_PATH_HPP

#include <silicium/relative_path.hpp>
#include <silicium/path_segment.hpp>
#include <silicium/optional.hpp>
#include <silicium/error_or.hpp>
#include <silicium/is_handle.hpp>
#include <silicium/c_string.hpp>
#include <silicium/get_last_error.hpp>
#ifdef _WIN32
#	include <silicium/win32/win32.hpp>
#endif

//Boost filesystem requires exceptions
#define SILICIUM_HAS_ABSOLUTE_PATH_OPERATIONS SILICIUM_HAS_EXCEPTIONS

#if SILICIUM_HAS_ABSOLUTE_PATH_OPERATIONS
#	include <boost/filesystem/operations.hpp>
#endif

#include <iostream>

namespace Si
{
	struct absolute_path
	{
		typedef native_path_char char_type;
		typedef path::underlying_type underlying_type;

		absolute_path() BOOST_NOEXCEPT
		{
		}

		absolute_path(absolute_path &&other) BOOST_NOEXCEPT
		    : m_value(std::move(other.m_value))
		{
		}

		absolute_path(absolute_path const &other)
			: m_value(other.m_value)
		{
		}

		absolute_path &operator = (absolute_path &&other) BOOST_NOEXCEPT
		{
			m_value = std::move(other.m_value);
			return *this;
		}

		absolute_path &operator = (absolute_path const &other)
		{
			m_value = other.m_value;
			return *this;
		}

		void swap(absolute_path &other) BOOST_NOEXCEPT
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
		noexcept_string const &
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
		native_path_string safe_c_str() const BOOST_NOEXCEPT
		{
			return native_path_string(c_str());
		}

		void combine(relative_path const &back)
		{
			//TODO: optimize
			*this = absolute_path(to_boost_path() / back.to_boost_path());
		}

		template <class RelativePath>
		absolute_path &operator /= (RelativePath &&other)
		{
			combine(std::forward<RelativePath>(other));
			return *this;
		}

		SILICIUM_USE_RESULT
		bool empty() const BOOST_NOEXCEPT
		{
			return underlying().empty();
		}

		SILICIUM_USE_RESULT
		static optional<absolute_path> create(boost::filesystem::path const &maybe_absolute)
		{
			if (maybe_absolute.is_absolute())
			{
				return absolute_path(maybe_absolute);
			}
			return none;
		}

		SILICIUM_USE_RESULT
		static optional<absolute_path> create(noexcept_string const &maybe_absolute)
		{
			return create(boost::filesystem::path(maybe_absolute.c_str()));
		}

		SILICIUM_USE_RESULT
		static optional<absolute_path> create(char const *maybe_absolute)
		{
			return create(boost::filesystem::path(maybe_absolute));
		}

#ifdef _WIN32
		SILICIUM_USE_RESULT
		static optional<absolute_path> create(wchar_t const *maybe_absolute)
		{
			return create(boost::filesystem::path(maybe_absolute));
		}
#endif

		optional<path_segment> name() const
		{
			auto &&path = to_boost_path();
			if (!path.has_filename())
			{
				return none;
			}
			return path_segment::create(path.filename());
		}

	private:

		path m_value;

		explicit absolute_path(boost::filesystem::path const &value)
			: m_value(value)
		{
		}
	};

#if SILICIUM_HAS_IS_HANDLE
	BOOST_STATIC_ASSERT(Si::is_handle<absolute_path>::value);
#endif

	inline std::ostream &operator << (std::ostream &out, absolute_path const &p)
	{
		return out << p.underlying();
	}

	inline std::istream &operator >> (std::istream &in, absolute_path &p)
	{
		absolute_path::underlying_type temp;
		in >> temp;
		if (!in)
		{
			return in;
		}
		optional<absolute_path> checked = absolute_path::create(std::move(temp));
		if (!checked)
		{
			in.setstate(std::ios::failbit);
			return in;
		}
		p = std::move(*checked);
		return in;
	}

	template <class ComparableToPath>
	SILICIUM_USE_RESULT
	inline bool operator == (absolute_path const &left, ComparableToPath const &right)
	{
		return left.underlying() == right;
	}

	template <class ComparableToPath>
	SILICIUM_USE_RESULT
	inline bool operator == (ComparableToPath const &left, absolute_path const &right)
	{
		return left == right.underlying();
	}

	SILICIUM_USE_RESULT
	inline bool operator == (absolute_path const &left, boost::filesystem::path const &right)
	{
		return right == left.c_str();
	}

	SILICIUM_USE_RESULT
	inline bool operator == (boost::filesystem::path const &left, absolute_path const &right)
	{
		return left == right.c_str();
	}

	SILICIUM_USE_RESULT
	inline bool operator == (absolute_path const &left, absolute_path const &right)
	{
		return left.underlying() == right.underlying();
	}

	template <class ComparableToPath>
	SILICIUM_USE_RESULT
	inline bool operator != (absolute_path const &left, ComparableToPath const &right)
	{
		return !(left == right);
	}

	template <class ComparableToPath>
	SILICIUM_USE_RESULT
	inline bool operator != (ComparableToPath const &left, absolute_path const &right)
	{
		return !(left == right);
	}

	SILICIUM_USE_RESULT
	inline bool operator < (absolute_path const &left, absolute_path const &right)
	{
		return left.underlying() < right.underlying();
	}

	SILICIUM_USE_RESULT
	inline std::size_t hash_value(absolute_path const &value)
	{
		using boost::hash_value;
		return hash_value(value.underlying());
	}

	SILICIUM_USE_RESULT
	inline relative_path leaf(absolute_path const &whole)
	{
		//TODO: do this efficiently
		return relative_path(whole.to_boost_path().leaf());
	}

	SILICIUM_USE_RESULT
	inline optional<absolute_path> parent(absolute_path const &whole)
	{
		//TODO: do this efficiently
		auto boosted = whole.to_boost_path();
		if (boosted.has_parent_path())
		{
			return *absolute_path::create(boosted.parent_path());
		}
		return none;
	}

	SILICIUM_USE_RESULT
	inline absolute_path operator / (absolute_path const &front, relative_path const &back)
	{
		//TODO: do this efficiently
		absolute_path result = front;
		result.combine(back);
		return result;
	}

	template <std::size_t N>
	SILICIUM_USE_RESULT
	inline absolute_path operator / (absolute_path const &front, absolute_path::char_type const (&literal)[N])
	{
		return front / relative_path(boost::filesystem::path(&literal[0]));
	}

#ifdef _WIN32
	template <std::size_t N>
	SILICIUM_USE_RESULT
	inline absolute_path operator / (absolute_path const &front, char const (&literal)[N])
	{
		return front / relative_path(boost::filesystem::path(&literal[0]));
	}
#endif

	inline absolute_path operator / (absolute_path const &front, path_segment const &back)
	{
		absolute_path result = front;
		result.combine(relative_path(back.to_boost_path()));
		return result;
	}

	inline noexcept_string to_utf8_string(boost::filesystem::path const &path)
	{
		return to_utf8_string(path.string());
	}

	inline noexcept_string to_utf8_string(absolute_path const &path)
	{
		return to_utf8_string(path.to_boost_path());
	}

#if SILICIUM_HAS_ABSOLUTE_PATH_OPERATIONS
	SILICIUM_USE_RESULT
	inline absolute_path get_current_working_directory()
	{
		return *absolute_path::create(boost::filesystem::current_path());
	}

	SILICIUM_USE_RESULT
	inline boost::system::error_code remove_file(absolute_path const &name)
	{
		boost::system::error_code ec;
		boost::filesystem::remove(name.to_boost_path(), ec);
		return ec;
	}

	SILICIUM_USE_RESULT
	inline boost::system::error_code create_directories(absolute_path const &directories)
	{
		boost::system::error_code ec;
		boost::filesystem::create_directories(directories.to_boost_path(), ec);
		return ec;
	}

	SILICIUM_USE_RESULT
	inline error_or<boost::uint64_t> remove_all(absolute_path const &directories)
	{
		boost::system::error_code ec;
		auto count = boost::filesystem::remove_all(directories.to_boost_path(), ec);
		if (!!ec)
		{
			return ec;
		}
		return count;
	}

	SILICIUM_USE_RESULT
	inline boost::system::error_code recreate_directories(absolute_path const &directories)
	{
		boost::system::error_code error = create_directories(directories);
		if (!!error)
		{
			return error;
		}
		boost::filesystem::directory_iterator i(directories.to_boost_path(), error);
		if (!!error)
		{
			return error;
		}
		for (; i != boost::filesystem::directory_iterator(); )
		{
			boost::filesystem::remove_all(i->path(), error);
			if (!!error)
			{
				return error;
			}
			i.increment(error);
			if (!!error)
			{
				return error;
			}
		}
		return error;
	}

	inline boost::system::error_code copy(absolute_path const &from, absolute_path const &to)
	{
		boost::system::error_code ec;
		boost::filesystem::copy(from.to_boost_path(), to.to_boost_path(), ec);
		return ec;
	}

	SILICIUM_USE_RESULT
	inline error_or<bool> file_exists(absolute_path const &file)
	{
		boost::system::error_code ec;
		boost::filesystem::file_status status = boost::filesystem::status(file.to_boost_path(), ec);
		if (status.type() == boost::filesystem::file_not_found)
		{
			return false;
		}
		if (ec)
		{
			return ec;
		}
		return true;
	}

	SILICIUM_USE_RESULT
	inline boost::system::error_code rename(absolute_path const &from, absolute_path const &to)
	{
		boost::system::error_code ec;
		boost::filesystem::rename(from.to_boost_path(), to.to_boost_path(), ec);
		return ec;
	}

	inline Si::error_or<Si::absolute_path> get_current_executable_path()
	{
#ifdef _WIN32
		//will be enough for most cases
		std::vector<wchar_t> buffer(MAX_PATH);
		for (;;)
		{
			auto const length = GetModuleFileNameW(NULL, buffer.data(), buffer.size());
			auto const error = Si::get_last_error();
			switch (error.value())
			{
			case ERROR_INSUFFICIENT_BUFFER:
				buffer.resize(buffer.size() * 2);
				break;

			case ERROR_SUCCESS:
			{
				boost::filesystem::path path(buffer.begin(), buffer.begin() + length);
				return *Si::absolute_path::create(std::move(path));
			}

			default:
				return error;
			}
		}
#else
		boost::system::error_code ec;
		auto result = boost::filesystem::read_symlink("/proc/self/exe", ec);
		if (!!ec)
		{
			return ec;
		}
		return *Si::absolute_path::create(std::move(result));
#endif
	}

	inline absolute_path temporary_directory()
	{
		return *absolute_path::create(boost::filesystem::temp_directory_path());
	}
#endif
}

namespace std
{
	template <>
	struct hash< ::Si::absolute_path>
	{
		SILICIUM_USE_RESULT
		std::size_t operator()(Si::absolute_path const &value) const
		{
			return hash_value(value);
		}
	};
}

#endif
