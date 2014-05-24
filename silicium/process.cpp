#include <silicium/process.hpp>
#include <silicium/to_unique.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/system/system_error.hpp>
#include <algorithm>
#include <memory>

#ifdef __linux__
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace Si
{
	namespace
	{
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

			void copy_all(int source, sink<char> &destination)
			{
				for (;;)
				{
					buffering_sink<char> buffer_helper(destination);
					auto const buffer = buffer_helper.make_append_space(std::numeric_limits<std::size_t>::max());
					assert(!buffer.empty());
					auto const rc = read(source, buffer.begin(), buffer.size());
					if (rc == 0)
					{
						break;
					}
					if (rc < 0)
					{
						throw boost::system::system_error(errno, boost::system::system_category());
					}
					commit(buffer_helper, static_cast<std::size_t>(rc));
				}
			}
		}
	}

	int run_process(process_parameters const &parameters)
	{
		auto executable = parameters.executable.string();
		auto arguments = parameters.arguments;
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

		if (parameters.stdout)
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
			if (parameters.stdout)
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

			boost::filesystem::current_path(parameters.current_path);

			execvp(parameters.executable.c_str(), argument_pointers.data());

			//kill the process in case execv fails
			std::abort();
		}

		//parent
		else
		{
			if (parameters.stdout)
			{
				stdout_writing.reset();
				detail::copy_all(*stdout_reading, *parameters.stdout);
			}
			stdin_reading.reset();

			int status = 0;
			if (waitpid(forked, &status, 0) < 0)
			{
				throw boost::system::system_error(errno, boost::system::system_category());
			}

			int const exit_status = WEXITSTATUS(status);
			return exit_status;
		}
	}

	process_results run_process(std::string executable, std::vector<std::string> arguments, boost::filesystem::path current_path)
	{
		process_parameters parameters;
		parameters.executable = std::move(executable);
		parameters.arguments = std::move(arguments);
		parameters.current_path = std::move(current_path);
		std::vector<char> stdout;
		parameters.stdout = to_unique(make_container_sink(stdout));
		auto const result = run_process(parameters);
		return process_results{result, std::move(stdout)};
	}
}
