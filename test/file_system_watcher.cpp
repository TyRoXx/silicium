#include <reactive/file_system_watcher.hpp>
#include <reactive/consume.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <fstream>

namespace rx
{
	namespace
	{
		void touch(boost::filesystem::path const &name)
		{
			std::ofstream file(name.string());
			file.close();
		}
	}

	BOOST_AUTO_TEST_CASE(file_system_watcher_add)
	{
		boost::filesystem::path const watched_dir = boost::filesystem::current_path();
		boost::filesystem::path const test_file = watched_dir / "test.txt";
		{
			boost::system::error_code ec;
			boost::filesystem::remove(test_file, ec);
		}

		boost::asio::io_service io;
		file_system_watcher watcher(io, watched_dir);

		bool got_event = false;
		auto consumer = consume<file_notification>([&test_file, &io, &got_event](file_notification const &event)
		{
			io.stop();
			BOOST_CHECK(file_notification_type::add == event.type);
			BOOST_CHECK(equivalent(test_file, event.name));
			got_event = true;
		});
		watcher.async_get_one(consumer);

		touch(test_file);

		boost::asio::steady_timer timeout(io);
		timeout.expires_from_now(std::chrono::seconds(1));
		timeout.async_wait([&io](boost::system::error_code)
		{
			io.stop();
		});

		io.run();

		BOOST_CHECK(got_event);
	}
}
