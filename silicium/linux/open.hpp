#ifndef SILICIUM_LINUX_OPEN_HPP
#define SILICIUM_LINUX_OPEN_HPP

#include <silicium/optional.hpp>
#include <silicium/linux/file_descriptor.hpp>
#include <boost/filesystem/path.hpp>
#include <fcntl.h>

namespace Si
{
	namespace linux
	{
		template <class Value, class Error = boost::system::error_code>
		struct error_or
		{
			error_or() BOOST_NOEXCEPT
			{
			}

			error_or(Value value) BOOST_NOEXCEPT
				: storage(std::move(value))
			{
			}

			error_or(Error error) BOOST_NOEXCEPT
				: storage(std::move(error))
			{
			}

			bool is_error() const BOOST_NOEXCEPT
			{
				return Si::visit<bool>(
							storage,
							[](Value const &) { return false; },
							[](Error const &) { return true; });
			}

			Si::optional<Error> error() const BOOST_NOEXCEPT
			{
				return Si::visit<Si::optional<Error>>(
							storage,
							[](Value const &) { return Si::optional<Error>(); },
							[](Error const &e) { return e; });
			}

			Value &value()
			{
				return Si::visit<Value &>(
							storage,
							[](Value &value) -> Value & { return value; },
							[](Error const &e) -> Value & { boost::throw_exception(boost::system::system_error(e)); });
			}

			Value const &value() const
			{
				return Si::visit<Value const &>(
							storage,
							[](Value const &value) -> Value const & { return value; },
							[](Error const &e) -> Value const & { boost::throw_exception(boost::system::system_error(e)); });
			}

		private:

			fast_variant<Value, Error> storage;
		};

		inline error_or<Si::linux::file_descriptor> open_reading(boost::filesystem::path const &name)
		{
			int const fd = ::open(name.c_str(), O_RDONLY);
			if (fd < 0)
			{
				return boost::system::error_code(errno, boost::system::system_category());
			}
			return Si::linux::file_descriptor(fd);
		}
	}
}

#endif
