#include <reactive/directory_watcher.hpp>
#include <reactive/for_each.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream>

int main()
{
	boost::asio::io_service io;

	auto const watched_dir = boost::filesystem::current_path();
	std::cerr << "Watching " << watched_dir << '\n';

	rx::directory_watcher notifier(io, watched_dir);
	auto all = rx::for_each(rx::ref(notifier), [](rx::file_notification const &event)
	{
		std::cerr << boost::underlying_cast<int>(event.type) << " " << event.name << '\n';
	});
	all.start();

	io.run();
}
