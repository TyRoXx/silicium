#include <reactive/enumerate.hpp>
#include <reactive/for_each.hpp>
#include <reactive/ref.hpp>
#ifdef _WIN32
#	include <reactive/win32/directory_changes.hpp>
#else
#	include <reactive/linux/inotify.hpp>
#endif
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/numeric.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream>

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

#ifdef _WIN32
	rx::win32::directory_changes notifier(watched_dir, false);
	auto all = rx::for_each(rx::enumerate(rx::ref(notifier)), [](rx::win32::file_notification const &event)
	{
		std::cerr << event.name << '\n';
	});
	boost::asio::io_service::work keep_running(io);
#else
	rx::linux::inotify_observable notifier(io);
	auto w = notifier.watch(watched_dir, IN_ALL_EVENTS);
	auto all = rx::for_each(rx::enumerate(rx::ref(notifier)), [](rx::linux::file_notification const &event)
	{
		std::cerr << inotify_mask_to_string(event.mask) << " " << event.name << '\n';
	});
#endif
	all.start();

	io.run();
}
