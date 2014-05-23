#include <silicium/git/repository.hpp>
#include <boost/filesystem/operations.hpp>

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
	Si::git::unique_repository repo2(repo);
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
