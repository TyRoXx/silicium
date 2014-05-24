#ifndef SILICIUM_GIT_REPOSITORY_HPP
#define SILICIUM_GIT_REPOSITORY_HPP

#include <git2.h>
#include <memory>
#include <boost/filesystem/path.hpp>

namespace Si
{
	namespace git
	{
		struct repository_deleter
		{
			void operator()(git_repository *repository);
		};
		typedef std::unique_ptr<git_repository, repository_deleter> unique_repository;

		struct reference_deleter
		{
			void operator()(git_reference *reference);
		};
		typedef std::unique_ptr<git_reference, reference_deleter> unique_reference;

		struct commit_deleter
		{
			void operator()(git_commit *commit);
		};
		typedef std::unique_ptr<git_commit, commit_deleter> unique_commit;

		struct git_error : std::runtime_error
		{
			explicit git_error(int code, std::string message);
			int code() const;

		private:

			int m_code;
		};


		void throw_if_libgit2_error(int error);
		unique_repository open_repository(boost::filesystem::path const &where);
		unique_repository clone(std::string const &source, boost::filesystem::path const &destination, git_clone_options const *options);
		unique_reference lookup(git_repository &repository, char const *name);
		unique_commit lookup_commit(git_repository &repository, git_oid const &id);
		std::string format_oid(git_oid const &id);
	}
}

#endif
