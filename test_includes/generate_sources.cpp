#include <boost/filesystem.hpp>
#include <fstream>

int main()
{
	boost::filesystem::path const this_source_file = __FILE__;
	boost::filesystem::path const test_includes_root = this_source_file.parent_path();
	boost::filesystem::path const sources_root = test_includes_root / "sources";
	boost::filesystem::path const headers_root = test_includes_root.parent_path() / "silicium";

	for (boost::filesystem::directory_iterator i(headers_root); i != boost::filesystem::directory_iterator(); ++i)
	{
		boost::filesystem::path const header_full = i->path();
		boost::filesystem::path const header_leaf = header_full.leaf();
		boost::filesystem::path const header_extension = header_leaf.extension();
		if (header_extension != ".hpp")
		{
			continue;
		}
		boost::filesystem::path const header_no_extension = header_leaf.stem();
		boost::filesystem::path const source = sources_root / (header_no_extension.string() + ".cpp");
		std::ofstream source_file(source.string());
		if (!source_file)
		{
			throw std::runtime_error("Cannot open file " + source.string());
		}
		source_file << "#include <silicium/" << header_leaf.string() << ">\n";
		if (!source_file)
		{
			throw std::runtime_error("Could not write to " + source.string());
		}
	}
}
