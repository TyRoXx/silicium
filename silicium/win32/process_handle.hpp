#ifndef SILICIUM_WIN32_PROCESS_HANDLE_HPP
#define SILICIUM_WIN32_PROCESS_HANDLE_HPP

#include <silicium/error_or.hpp>
#include <silicium/exchange.hpp>
#include <silicium/win32/win32.hpp>
#include <boost/swap.hpp>

namespace Si
{
	struct process_handle
	{
		process_handle() BOOST_NOEXCEPT
			: m_id(INVALID_HANDLE_VALUE)
		{
		}

		explicit process_handle(HANDLE id) BOOST_NOEXCEPT
			: m_id(id)
		{
		}

		~process_handle() BOOST_NOEXCEPT
		{
			if (m_id  == INVALID_HANDLE_VALUE)
			{
				return;
			}
			wait_for_exit();
		}

		process_handle(process_handle &&other) BOOST_NOEXCEPT
			: m_id(INVALID_HANDLE_VALUE)
		{
			swap(other);
		}

		process_handle &operator = (process_handle &&other) BOOST_NOEXCEPT
		{
			swap(other);
			return *this;
		}

		void swap(process_handle &other) BOOST_NOEXCEPT
		{
			boost::swap(m_id, other.m_id);
		}

		error_or<int> wait_for_exit() BOOST_NOEXCEPT
		{
			WaitForSingleObject(m_id, INFINITE);
			DWORD exit_code = 1;
			if (!GetExitCodeProcess(m_id, &exit_code))
			{
				return boost::system::error_code(::GetLastError(), boost::system::native_ecat);
			}
			CloseHandle(m_id);
			m_id = INVALID_HANDLE_VALUE;
			return static_cast<int>(exit_code);
		}

	private:

		HANDLE m_id;

		SILICIUM_DELETED_FUNCTION(process_handle(process_handle const &))
		SILICIUM_DELETED_FUNCTION(process_handle &operator = (process_handle const &))
	};
}

#endif
