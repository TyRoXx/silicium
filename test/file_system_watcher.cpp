#include <reactive/directory_watcher.hpp>
#include <reactive/consume.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <fstream>

namespace rx
{
	namespace
	{
		void touch(boost::filesystem::path const &name)
		{
			std::ofstream file(name.string());
			BOOST_REQUIRE(file);
		}

		void test_single_event(
			boost::filesystem::path const &watched_dir,
			std::function<void ()> const &provoke_event,
			std::function<void (file_notification const &)> const &on_event)
		{
			boost::asio::io_service io;
			directory_watcher watcher(io, watched_dir);

			bool got_event = false;
			auto consumer = consume<file_notification>([&io, &got_event, &on_event](file_notification const &event)
			{
				io.stop();
				on_event(event);
				got_event = true;
			});
			watcher.async_get_one(consumer);

#ifdef _WIN32
			//currently the implementation for Windows needs some time for the background thread to set everything up
			boost::this_thread::sleep(boost::posix_time::seconds(1));
#endif

			provoke_event();

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

	BOOST_AUTO_TEST_CASE(file_system_watcher_add)
	{
		boost::filesystem::path const watched_dir = boost::filesystem::current_path();
		boost::filesystem::path const test_file = watched_dir / "test.txt";
		{
			boost::system::error_code ec;
			boost::filesystem::remove(test_file, ec);
		}

		test_single_event(
			watched_dir,
			std::bind(touch, test_file),
			[&test_file](file_notification const &event)
		{

			BOOST_CHECK(file_notification_type::add == event.type);
			BOOST_CHECK(equivalent(test_file, event.name));
		});
	}

	BOOST_AUTO_TEST_CASE(file_system_watcher_remove)
	{
		boost::filesystem::path const watched_dir = boost::filesystem::current_path();
		boost::filesystem::path const test_file = watched_dir / "test.txt";

		touch(test_file);

		test_single_event(
			watched_dir,
			[&test_file]
		{
			boost::filesystem::remove(test_file);
		},
			[&test_file, &watched_dir](file_notification const &event)
		{
			BOOST_CHECK(file_notification_type::remove == event.type);
			BOOST_CHECK_EQUAL(test_file, watched_dir / event.name);
		});
	}

	BOOST_AUTO_TEST_CASE(file_system_watcher_change)
	{
		boost::filesystem::path const watched_dir = boost::filesystem::current_path();
		boost::filesystem::path const test_file = watched_dir / "test.txt";

		touch(test_file);

		test_single_event(
			watched_dir,
			[&test_file]
		{
			std::ofstream file(test_file.string());
			BOOST_REQUIRE(file);
			file << "hello\n";
		},
			[&test_file, &watched_dir](file_notification const &event)
		{
			BOOST_CHECK(file_notification_type::change == event.type);
			BOOST_CHECK_EQUAL(test_file, watched_dir / event.name);
		});
	}
}
