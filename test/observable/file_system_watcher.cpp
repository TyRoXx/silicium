#include <silicium/directory_watcher.hpp>
#include <silicium/observable/consume.hpp>
#include <silicium/observable/spawn_coroutine.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <fstream>

namespace Si
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
			watcher.async_get_one(Si::observe_by_ref(consumer));

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
			BOOST_CHECK(equivalent(test_file, event.name.to_boost_path()));
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
			BOOST_CHECK_EQUAL(test_file, watched_dir / event.name.to_boost_path());
		});
	}

	BOOST_AUTO_TEST_CASE(file_system_watcher_change_on_write)
	{
		boost::filesystem::path const watched_dir = boost::filesystem::current_path();
		boost::filesystem::path const test_file = watched_dir / "test.txt";

		touch(test_file);

		std::ofstream file(test_file.string());
		BOOST_REQUIRE(file);

		test_single_event(
			watched_dir,
			[&file]
		{
			file << "hello\n";
			file.flush();
			//we write to the file without closing it
		},
			[&test_file, &watched_dir](file_notification const &event)
		{
			BOOST_CHECK(file_notification_type::change == event.type);
			BOOST_CHECK_EQUAL(test_file, watched_dir / event.name.to_boost_path());
		});
	}

	BOOST_AUTO_TEST_CASE(file_system_watcher_change_on_close)
	{
		boost::filesystem::path const watched_dir = boost::filesystem::current_path();
		boost::filesystem::path const test_file = watched_dir / "test.txt";

		touch(test_file);

		std::ofstream file(test_file.string());
		BOOST_REQUIRE(file);
		file << "hello\n";
		file.flush();

		test_single_event(
			watched_dir,
			[&file]
		{
			//The closing of the file is expected to trigger a change event.
			file.close();
		},
			[&test_file, &watched_dir](file_notification const &event)
		{
			BOOST_CHECK(file_notification_type::change == event.type);
			BOOST_CHECK_EQUAL(test_file, watched_dir / event.name.to_boost_path());
		});
	}

	BOOST_AUTO_TEST_CASE(file_system_watcher_rename)
	{
		boost::filesystem::path const watched_dir = boost::filesystem::current_path();
		boost::filesystem::path const test_file_a = watched_dir / "test_a.txt";
		boost::filesystem::path const test_file_b = watched_dir / "test_b.txt";

		touch(test_file_a);
		boost::filesystem::remove(test_file_b);

		boost::asio::io_service io;
		directory_watcher watcher(io, watched_dir);

		bool got_something = false;

		spawn_coroutine([&watcher, &test_file_a, &test_file_b, &got_something](spawn_context &yield)
		{
			optional<file_notification> first_event = yield.get_one(Si::ref(watcher));
			BOOST_REQUIRE(first_event);
			optional<file_notification> second_event = yield.get_one(Si::ref(watcher));
			BOOST_REQUIRE(second_event);

			got_something = true;

			file_notification *remove = &*first_event;
			file_notification *add = &*second_event;
			if (add->type != file_notification_type::add)
			{
				using std::swap;
				swap(remove, add);
			}

			BOOST_CHECK(file_notification_type::remove == remove->type);
			BOOST_CHECK_EQUAL(test_file_a.leaf(), remove->name.to_boost_path());

			BOOST_CHECK(file_notification_type::add == add->type);
			BOOST_CHECK_EQUAL(test_file_b.leaf(), add->name.to_boost_path());
		});

		boost::filesystem::rename(test_file_a, test_file_b);
		io.run();

		BOOST_CHECK(got_something);
	}
}
