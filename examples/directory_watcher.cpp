#include <ventura/single_directory_watcher.hpp>
#include <silicium/observable/for_each.hpp>
#include <ventura/file_operations.hpp>
#include <silicium/terminate_on_exception.hpp>
#include <iostream>

int main()
{
#if VENTURA_HAS_SINGLE_DIRECTORY_WATCHER && SILICIUM_HAS_FOR_EACH_OBSERVABLE
	boost::asio::io_service io;
	auto const watched_dir = ventura::get_current_working_directory(Si::throw_);
	std::cerr << "Watching " << watched_dir << '\n';

	ventura::single_directory_watcher notifier(io, watched_dir);
	auto all = Si::for_each(Si::ref(notifier), [](Si::error_or<ventura::file_notification> const &event)
	{
		if (event.is_error())
		{
			std::cerr << "Something went wrong while watching: " << event.error() << '\n';
			return;
		}
		std::cerr <<
#if BOOST_VERSION >= 105000
			boost::underlying_cast
#else
			static_cast
#endif
			<int>(event.get().type) << " " << event.get().name << '\n';
	});
	all.start();
	io.run();
#else
	std::cerr << "This example requires Boost filesystem and/or a more recent compiler\n";
#endif
}
