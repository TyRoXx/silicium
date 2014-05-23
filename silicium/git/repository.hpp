#ifndef SILICIUM_BUILD_RESULT_HPP
#define SILICIUM_BUILD_RESULT_HPP

#include <git2.h>
#include <memory>

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

		typedef std::unique_ptr<git_repository, repository_deleter> unique_repository;
	}
}

#endif
