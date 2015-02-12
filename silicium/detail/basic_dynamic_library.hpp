#ifndef SILICIUM_DETAIL_BASIC_DYNAMIC_LIBRARY_HPP
#define SILICIUM_DETAIL_BASIC_DYNAMIC_LIBRARY_HPP

#include <silicium/c_string.hpp>
#include <boost/system/system_error.hpp>
#include <boost/throw_exception.hpp>

namespace Si
{
	namespace detail
	{
		template <class DynamicLibraryImpl>
		struct basic_dynamic_library : private DynamicLibraryImpl
		{
			basic_dynamic_library() BOOST_NOEXCEPT
			{
			}

			explicit basic_dynamic_library(c_string const &file)
			{
				open(file);
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			basic_dynamic_library(basic_dynamic_library &&) BOOST_NOEXCEPT = default;
			basic_dynamic_library &operator = (basic_dynamic_library &&) BOOST_NOEXCEPT = default;
#else
			basic_dynamic_library(basic_dynamic_library &&other) BOOST_NOEXCEPT
				: handle(std::move(other.handle))
			{
			}

			basic_dynamic_library &operator = (basic_dynamic_library &&other) BOOST_NOEXCEPT
			{
				handle = std::move(other.handle);
				return *this;
			}
#endif
			~basic_dynamic_library() BOOST_NOEXCEPT
			{
			}

			void open(c_string const &file, boost::system::error_code &ec)
			{
				std::unique_ptr<void, deleter> new_handle(DynamicLibraryImpl::open(file, ec));
				if (!ec)
				{
					handle = std::move(new_handle);
				}
			}

			void open(c_string const &file)
			{
				boost::system::error_code ec;
				open(file, ec);
				if (ec)
				{
					boost::throw_exception(boost::system::system_error(ec));
				}
			}

			void *find_symbol(c_string const &name) const
			{
				return DynamicLibraryImpl::find_symbol(handle.get(), name);
			}

			bool empty() const BOOST_NOEXCEPT
			{
				return !handle;
			}

		private:

			struct deleter
			{
				void operator()(void *handle) const BOOST_NOEXCEPT
				{
					assert(handle);
					DynamicLibraryImpl::close(handle);
				}
			};

			std::unique_ptr<void, deleter> handle;

			SILICIUM_DELETED_FUNCTION(basic_dynamic_library(basic_dynamic_library const &))
			SILICIUM_DELETED_FUNCTION(basic_dynamic_library &operator = (basic_dynamic_library const &))
		};
	}
}

#endif
