#ifndef SILICIUM_LINUX_PROCESS_HANDLE_HPP
#define SILICIUM_LINUX_PROCESS_HANDLE_HPP

#include <silicium/error_or.hpp>
#include <silicium/exchange.hpp>
#include <sys/wait.h>

namespace Si
{
	struct process_handle
	{
		process_handle() BOOST_NOEXCEPT
			: m_id(-1)
		{
		}

		explicit process_handle(pid_t id) BOOST_NOEXCEPT
			: m_id(id)
		{
		}

		~process_handle() BOOST_NOEXCEPT
		{
			if (m_id < 0)
			{
				return;
			}
			wait_for_exit();
		}

		process_handle(process_handle &&other) BOOST_NOEXCEPT
			: m_id(-1)
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
			int status = 0;
			int wait_id = Si::exchange(m_id, -1);
			assert(wait_id >= 1);
			if (waitpid(wait_id, &status, 0) < 0)
			{
				return boost::system::error_code(errno, boost::system::system_category());
			}
			int const exit_status = WEXITSTATUS(status);
			return exit_status;
		}

	private:

		pid_t m_id;

		SILICIUM_DELETED_FUNCTION(process_handle(process_handle const &))
		SILICIUM_DELETED_FUNCTION(process_handle &operator = (process_handle const &))
	};
}

#endif
