#include <silicium/single_directory_watcher.hpp>
#include <silicium/observable/for_each.hpp>
#include <silicium/absolute_path.hpp>
#include <silicium/terminate_on_exception.hpp>
#include <iostream>

int main()
{
	boost::asio::io_service io;

#if SILICIUM_HAS_ABSOLUTE_PATH
	auto const watched_dir = Si::get_current_working_directory();
	std::cerr << "Watching " << watched_dir << '\n';

	Si::single_directory_watcher notifier(io, watched_dir);
	auto all = Si::for_each(Si::ref(notifier), [](Si::file_notification const &event)
	{
		std::cerr <<
#if BOOST_VERSION >= 105000
			boost::underlying_cast
#else
			static_cast
#endif
			<int>(event.type) << " " << event.name << '\n';
	});
	all.start();
#else
	std::cerr << "This example requires Boost filesystem support\n";
#endif

	io.run();
}
