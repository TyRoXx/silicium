#include <silicium/single_directory_watcher.hpp>
#include <silicium/observable/for_each.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream>

int main()
{
	boost::asio::io_service io;

	auto const watched_dir = boost::filesystem::current_path();
	std::cerr << "Watching " << watched_dir << '\n';

	Si::single_directory_watcher notifier(io, Si::absolute_path(watched_dir));
	auto all = Si::for_each(Si::ref(notifier), [](Si::file_notification const &event)
	{
		std::cerr << boost::underlying_cast<int>(event.type) << " " << event.name << '\n';
	});
	all.start();

	io.run();
}
