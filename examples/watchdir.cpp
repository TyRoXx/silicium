#include <reactive/enumerate.hpp>
#include <reactive/for_each.hpp>
#include <reactive/ref.hpp>
#include <reactive/transform_if_initialized.hpp>
#include <reactive/file_system_watcher.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/numeric.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/optional.hpp>
#include <iostream>
#ifndef _WIN32
#	include <reactive/linux/inotify.hpp>
#endif

namespace
{
#ifndef _WIN32
	std::string inotify_mask_to_string(boost::uint32_t mask)
	{
		using flag_entry = std::pair<boost::uint32_t, std::string>;
		static std::array<flag_entry, 12> const flag_names
		{{
			{IN_ACCESS, "access"},
			{IN_MODIFY, "modify"},
			{IN_ATTRIB, "attrib"},
			{IN_CLOSE_WRITE, "close_write"},
			{IN_CLOSE_NOWRITE, "close_nowrite"},
			{IN_OPEN, "open"},
			{IN_MOVED_FROM, "moved_from"},
			{IN_MOVED_TO, "moved_to"},
			{IN_CREATE, "create"},
			{IN_DELETE, "delete"},
			{IN_DELETE_SELF, "delete_self"},
			{IN_MOVE_SELF, "move_self"}
		}};
		return boost::accumulate(flag_names | boost::adaptors::filtered([mask](flag_entry const &entry)
		{
			return (entry.first & mask) == entry.first;
		}), std::string(), [](std::string left, flag_entry const &flag)
		{
			if (!left.empty())
			{
				left += "|";
			}
			left += flag.second;
			return left;
		});
	}
#endif
}

int main()
{
	boost::asio::io_service io;

	auto const watched_dir = boost::filesystem::current_path();
	std::cerr << "Watching " << watched_dir << '\n';

	rx::file_system_watcher notifier(io, watched_dir);
	auto all = rx::for_each(rx::ref(notifier), [](rx::file_notification const &event)
	{
		std::cerr << boost::underlying_cast<int>(event.type) << " " << event.name << '\n';
	});
	all.start();

	io.run();
}
