#ifndef SILICIUM_FILE_OPERATIONS_HPP
#define SILICIUM_FILE_OPERATIONS_HPP

#include <silicium/run_process.hpp>
#include <silicium/identity.hpp>
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

	namespace detail
	{
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
			for (; i != boost::filesystem::directory_iterator();)
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
	}

	template <class ErrorHandler>
	auto recreate_directories(absolute_path const &directories, ErrorHandler &&handle_error)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> decltype(std::forward<ErrorHandler>(handle_error)(boost::declval<boost::system::error_code>(), identity<void>()))
#endif
	{
		boost::system::error_code error = detail::recreate_directories(directories);
		return std::forward<ErrorHandler>(handle_error)(error, identity<void>());
	}

	namespace detail
	{
		struct returning_error_handler
		{
			boost::system::error_code operator()(boost::system::error_code error, identity<void>) const
			{
				return error;
			}
		};

		struct throwing_error_handler
		{
			template <class Result>
			Result operator()(boost::system::error_code error, identity<Result>) const
			{
				throw_error(error);
			}

			void operator()(boost::system::error_code error, identity<void>) const
			{
				if (!error)
				{
					return;
				}
				throw_error(error);
			}
		};

		struct variant_error_handler
		{
			template <class Result>
			error_or<Result> operator()(boost::system::error_code error, identity<Result>) const
			{
				return error;
			}
		};
	}

	static detail::returning_error_handler const return_;
	static detail::throwing_error_handler const throw_;
	static detail::variant_error_handler const variant_;

	SILICIUM_USE_RESULT
	inline boost::system::error_code copy(absolute_path const &from, absolute_path const &to)
	{
		boost::system::error_code ec;
		boost::filesystem::copy(from.to_boost_path(), to.to_boost_path(), ec);
		return ec;
	}

	template <class ErrorHandler>
	inline auto copy_recursively(
		absolute_path const &from,
		absolute_path const &to,
		Sink<char, success>::interface *output,
		ErrorHandler &&handle_error
	)
		-> decltype(std::forward<ErrorHandler>(handle_error)(boost::declval<boost::system::error_code>(), identity<void>()))
	{
#ifdef _WIN32
		boost::ignore_unused_variable_warning(from);
		boost::ignore_unused_variable_warning(to);
		boost::ignore_unused_variable_warning(output);
		throw std::logic_error("copy_recursively: not implemented");
#else
		std::vector<os_string> arguments;
		arguments.push_back(SILICIUM_SYSTEM_LITERAL("-Rv"));
		arguments.push_back(from.c_str());
		arguments.push_back(to.c_str());
		auto null_output = Sink<char, success>::erase(null_sink<char, success>());
		if (!output)
		{
			output = &null_output;
		}
		error_or<int> result = run_process(*Si::absolute_path::create("/bin/cp"), arguments, from, *output);
		if (!result.is_error() && (result.get() != 0))
		{
			throw std::runtime_error("cp failed"); //TODO: return a custom error_code for that
		}
		return std::forward<ErrorHandler>(handle_error)(result.error(), identity<void>());
#endif
	}

	template <class ErrorHandler>
	SILICIUM_USE_RESULT
	inline auto file_exists(absolute_path const &file, ErrorHandler &&handle_error)
		-> decltype(std::forward<ErrorHandler>(handle_error)(boost::declval<boost::system::error_code>(), identity<bool>()))
	{
		boost::system::error_code ec;
		boost::filesystem::file_status status = boost::filesystem::status(file.to_boost_path(), ec);
		if (status.type() == boost::filesystem::file_not_found)
		{
			return false;
		}
		if (ec)
		{
			return std::forward<ErrorHandler>(handle_error)(ec, identity<bool>());
		}
		return true;
	}

	template <class ErrorHandler>
	inline auto rename(absolute_path const &from, absolute_path const &to, ErrorHandler &&handle_error)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> decltype(std::forward<ErrorHandler>(handle_error)(boost::declval<boost::system::error_code>(), identity<void>()))
#endif
	{
		boost::system::error_code ec;
		boost::filesystem::rename(from.to_boost_path(), to.to_boost_path(), ec);
		return std::forward<ErrorHandler>(handle_error)(ec, identity<void>());
	}

	template <class ErrorHandler>
	inline auto get_current_executable_path(ErrorHandler &&handle_error)
		-> decltype(std::forward<ErrorHandler>(handle_error)(boost::declval<boost::system::error_code>(), identity<absolute_path>()))
	{
#ifdef _WIN32
		//will be enough for most cases
		std::vector<wchar_t> buffer(MAX_PATH);
		for (;;)
		{
			auto const length = GetModuleFileNameW(NULL, buffer.data(), buffer.size());
			auto const ec = Si::get_last_error();
			switch (ec.value())
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
				return std::forward<ErrorHandler>(handle_error)(ec, identity<absolute_path>());
			}
		}
#else
		boost::system::error_code ec;
		auto result = boost::filesystem::read_symlink("/proc/self/exe", ec);
		if (!!ec)
		{
			return std::forward<ErrorHandler>(handle_error)(ec, identity<absolute_path>());
		}
		return *Si::absolute_path::create(std::move(result));
#endif
	}

	template <class ErrorHandler>
	inline auto temporary_directory(ErrorHandler &&handle_error)
		-> decltype(std::forward<ErrorHandler>(handle_error)(boost::declval<boost::system::error_code>(), identity<absolute_path>()))
	{
		boost::system::error_code ec;
		auto temp = boost::filesystem::temp_directory_path(ec);
		if (!!ec)
		{
			return std::forward<ErrorHandler>(handle_error)(ec, identity<absolute_path>());
		}
		return *absolute_path::create(std::move(temp));
	}
#endif
}

#endif
