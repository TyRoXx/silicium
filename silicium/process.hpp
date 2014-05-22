#ifndef SILICIUM_PROCESS_HPP
#define SILICIUM_PROCESS_HPP

#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <boost/system/system_error.hpp>
#include <boost/optional.hpp>
#include <boost/filesystem/operations.hpp>

#ifdef __linux__
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace Si
{
	struct process_output
	{
		int exit_status;
		//TODO: make stdout asynchronously readable
		boost::optional<std::vector<char>> stdout;
	};

#ifdef __linux__
	namespace detail
	{
		void terminating_close(int file) noexcept
		{
			if (close(file) < 0)
			{
				//it is intended that this will terminate the process because of noexcept
				throw boost::system::system_error(errno, boost::system::system_category());
			}
		}

		struct file_closer
		{
			void operator()(int *file) const noexcept
			{
				assert(file);
				terminating_close(*file);
			}
		};

		std::vector<char> read_all(int source)
		{
			std::vector<char> content;
			std::size_t used_content = 0;
			std::size_t const buffer_size = 1U << 14U;
			for (;;)
			{
				content.resize(used_content + buffer_size);
				auto const rc = read(source, content.data() + used_content, buffer_size);
				if (rc == 0)
				{
					//end of file
					break;
				}
				if (rc < 0)
				{
					throw boost::system::system_error(errno, boost::system::system_category());
				}
				assert(static_cast<std::size_t>(rc) <= buffer_size);
				used_content += static_cast<std::size_t>(rc);
			}
			content.resize(used_content);
			return content;
		}
	}

	process_output run_process(std::string executable, std::vector<std::string> arguments, boost::filesystem::path const &current_path, bool dump_stdout)
	{
		std::vector<char *> argument_pointers;
		argument_pointers.emplace_back(&executable[0]);
		std::transform(begin(arguments), end(arguments), std::back_inserter(argument_pointers), [](std::string &arg)
		{
			return &arg[0];
		});
		argument_pointers.emplace_back(nullptr);

		std::array<int, 2> stdout;
		std::unique_ptr<int, detail::file_closer> stdout_reading;
		std::unique_ptr<int, detail::file_closer> stdout_writing;

		if (dump_stdout)
		{
			if (pipe(stdout.data()) < 0)
			{
				throw boost::system::system_error(errno, boost::system::system_category());
			}
			stdout_reading.reset(stdout.data() + 0);
			stdout_writing.reset(stdout.data() + 1);
		}

		std::array<int, 2> stdin;
		std::unique_ptr<int, detail::file_closer> stdin_reading;
		std::unique_ptr<int, detail::file_closer> stdin_writing;
		if (pipe(stdin.data()) < 0)
		{
			throw boost::system::system_error(errno, boost::system::system_category());
		}
		stdin_reading.reset(stdin.data() + 0);
		stdin_writing.reset(stdin.data() + 1);

		pid_t const forked = fork();
		if (forked < 0)
		{
			throw boost::system::system_error(errno, boost::system::system_category());
		}

		//child
		if (forked == 0)
		{
			if (dump_stdout)
			{
				if (dup2(*stdout_writing, STDOUT_FILENO) < 0)
				{
					std::abort();
				}
				if (dup2(*stdout_writing, STDERR_FILENO) < 0)
				{
					std::abort();
				}
				stdout_writing.reset();
				stdout_reading.reset();
			}

			if (dup2(*stdin_reading, STDIN_FILENO) < 0)
			{
				std::abort();
			}
			stdin_reading.reset();
			stdin_writing.reset();

			boost::filesystem::current_path(current_path);

			execvp(executable.c_str(), argument_pointers.data());

			//kill the process in case execv fails
			std::abort();
		}

		//parent
		else
		{
			boost::optional<std::vector<char>> all_stdout;
			if (dump_stdout)
			{
				stdout_writing.reset();
				all_stdout = detail::read_all(*stdout_reading);
			}
			stdin_reading.reset();

			int status = 0;
			if (waitpid(forked, &status, 0) < 0)
			{
				throw boost::system::system_error(errno, boost::system::system_category());
			}

			int const exit_status = WEXITSTATUS(status);
			return process_output{exit_status, std::move(all_stdout)};
		}
	}
#endif
}

#endif
