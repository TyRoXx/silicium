#include <silicium/process.hpp>
#include <git2.h>

namespace Si
{
	namespace git
	{
		struct repository_deleter
		{
			void operator()(git_repository *repository)
			{
				git_repository_free(repository);
			}
		};
	}
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		return 1;
	}

	git_repository *repo = nullptr;
	const char *url = argv[1];
	auto const destination = (boost::filesystem::current_path() / "si.git").string();
	int const error = git_clone(&repo, url, destination.c_str(), NULL);
	std::unique_ptr<git_repository, Si::git::repository_deleter> repo2(repo);
	repo = nullptr;
	if (error)
	{
		auto * const details = giterr_last();
		if (details)
		{
			throw std::runtime_error(details->message);
		}
		else
		{
			throw std::runtime_error("Unknown libgit2 error");
		}
	}
}
