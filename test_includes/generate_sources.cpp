#include <boost/filesystem.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <fstream>
#include <unordered_set>

namespace
{
	template <class PathPredicate, class PathHandler>
	void generate_sources_recursively(
		boost::filesystem::path const &sources,
		boost::filesystem::path const &headers,
		boost::filesystem::path const &relative_include_path,
		PathPredicate const &is_relevant_directory,
		PathHandler const &on_include
		)
	{
		for (boost::filesystem::directory_iterator i(headers); i != boost::filesystem::directory_iterator(); ++i)
		{
			switch (i->status().type())
			{
			case boost::filesystem::directory_file:
				if (is_relevant_directory(i->path()))
				{
					generate_sources_recursively(sources / i->path().leaf(), i->path(), relative_include_path / i->path().leaf(), is_relevant_directory, on_include);
				}
				break;

			case boost::filesystem::regular_file:
				{
					boost::filesystem::path const header_full = i->path();
					boost::filesystem::path const header_leaf = header_full.leaf();
					boost::filesystem::path const header_extension = header_leaf.extension();
					if (header_extension != ".hpp")
					{
						break;
					}
					boost::filesystem::path const header_no_extension = header_leaf.stem();
					boost::filesystem::create_directories(sources);
					boost::filesystem::path const source = sources / (header_no_extension.string() + ".cpp");
					std::string header_name = "<silicium/" + boost::algorithm::replace_all_copy((relative_include_path / header_leaf).string(), "\\", "/") + ">";
					on_include(header_name);
					if (boost::filesystem::exists(source))
					{
						break;
					}
					std::ofstream source_file(source.string());
					if (!source_file)
					{
						throw std::runtime_error("Cannot open file " + source.string());
					}
					source_file << "#include " << header_name << "\n";
					if (!source_file)
					{
						throw std::runtime_error("Could not write to " + source.string());
					}
					break;
				}

			default:
				break;
			}
		}
	}
}

int main()
{
	boost::filesystem::path const this_source_file = __FILE__;
	boost::filesystem::path const test_includes_root = this_source_file.parent_path();
	boost::filesystem::path const sources_root = test_includes_root / "sources";
	boost::filesystem::path const headers_root = test_includes_root.parent_path() / "silicium";
	auto const is_relevant_directory = [](boost::filesystem::path const &dir)
	{
		static std::unordered_set<std::string> const blacklist
		{
#ifndef _WIN32
			"win32",
#endif
#ifndef __linux__
			"linux",
#endif
		};
		return blacklist.count(dir.leaf().string()) == 0;
	};

	auto const all_headers_file_name = sources_root / "_all_headers.cpp";
	std::ofstream all_headers(all_headers_file_name.string());
	if (!all_headers)
	{
		throw std::runtime_error("Cannot open file " + all_headers_file_name.string());
	}
	auto const write_header = [&all_headers](std::string const &header_name)
	{
		all_headers << "#include " << header_name << '\n';
	};
	generate_sources_recursively(sources_root, headers_root, "", is_relevant_directory, write_header);
}
