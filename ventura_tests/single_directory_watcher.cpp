#include <ventura/single_directory_watcher.hpp>
#include <silicium/observable/consume.hpp>
#include <silicium/observable/spawn_coroutine.hpp>
#include <ventura/open.hpp>
#include <silicium/steady_clock.hpp>
#include <ventura/file_operations.hpp>
#include <ventura/sink/file_sink.hpp>
#include <silicium/sink/append.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <fstream>
#include <chrono>
#if VENTURA_HAS_SINGLE_DIRECTORY_WATCHER
#	include <boost/filesystem/operations.hpp>
#endif

namespace Si
{
#if VENTURA_HAS_SINGLE_DIRECTORY_WATCHER
	namespace
	{
		void touch(ventura::absolute_path const &name)
		{
			//remove existing file to reset the attributes
			ventura::remove_file(name, Si::throw_);

			std::ofstream file(name.c_str());
			BOOST_REQUIRE(file);
		}

		void test_single_event(
			ventura::absolute_path const &watched_dir,
			std::function<void ()> const &provoke_event,
			std::function<void (ventura::file_notification const &)> const &on_event)
		{
			boost::asio::io_service io;
			ventura::single_directory_watcher watcher(io, watched_dir);

			bool got_event = false;
			auto consumer = consume<error_or<ventura::file_notification>>([&io, &got_event, &on_event](error_or<ventura::file_notification> const &event)
			{
				io.stop();
				on_event(event.get());
				got_event = true;
			});
			watcher.async_get_one(Si::observe_by_ref(consumer));

			provoke_event();

			boost::asio::basic_waitable_timer<Si::steady_clock_if_available> timeout(io);
			timeout.expires_from_now(Si::chrono::seconds(1));
			timeout.async_wait([&io](boost::system::error_code ec)
			{
				BOOST_REQUIRE(!ec);
				io.stop();
			});

			io.run();

			BOOST_CHECK(got_event);
		}
	}

	BOOST_AUTO_TEST_CASE(file_system_watcher_add_regular)
	{
		ventura::absolute_path const watched_dir = ventura::get_current_working_directory(Si::throw_);
		ventura::absolute_path const test_file = watched_dir / ventura::relative_path("test.txt");
		ventura::remove_file(test_file, Si::throw_);

		test_single_event(
			watched_dir,
			std::bind(touch, test_file),
			[&test_file](ventura::file_notification const &event)
		{
			BOOST_CHECK(ventura::file_notification_type::add == event.type);
			BOOST_CHECK(equivalent(test_file.to_boost_path(), event.name.to_boost_path()));
		});
	}

	BOOST_AUTO_TEST_CASE(file_system_watcher_add_directory)
	{
		ventura::absolute_path const watched_dir = ventura::get_current_working_directory(Si::throw_);
		ventura::absolute_path const test_file = watched_dir / "test";
		ventura::remove_file(test_file, Si::throw_);

		test_single_event(
			watched_dir,
			[&test_file]
		{
			boost::filesystem::create_directory(test_file.to_boost_path());
		},
			[&test_file](ventura::file_notification const &event)
		{
			BOOST_CHECK(ventura::file_notification_type::add == event.type);
			BOOST_CHECK(equivalent(test_file.to_boost_path(), event.name.to_boost_path()));
		});
	}

	BOOST_AUTO_TEST_CASE(file_system_watcher_remove)
	{
		ventura::absolute_path const watched_dir = ventura::get_current_working_directory(Si::throw_);
		ventura::absolute_path const test_file = watched_dir / "test.txt";

		touch(test_file);

		test_single_event(
			watched_dir,
			[&test_file]
		{
			boost::filesystem::remove(test_file.to_boost_path());
		},
			[&test_file, &watched_dir](ventura::file_notification const &event)
		{
			BOOST_CHECK(ventura::file_notification_type::remove == event.type);
			BOOST_CHECK_EQUAL(test_file, watched_dir / event.name);
		});
	}

	BOOST_AUTO_TEST_CASE(file_system_watcher_remove_directory)
	{
		ventura::absolute_path const watched_dir = ventura::get_current_working_directory(Si::throw_);
		ventura::absolute_path const test_file = watched_dir / "test";

		boost::filesystem::create_directory(test_file.to_boost_path());

		test_single_event(
			watched_dir,
			[&test_file]
		{
			boost::filesystem::remove(test_file.to_boost_path());
		},
			[&test_file, &watched_dir](ventura::file_notification const &event)
		{
			BOOST_CHECK(ventura::file_notification_type::remove == event.type);
			BOOST_CHECK_EQUAL(test_file, watched_dir / event.name);
		});
	}


#if SILICIUM_HAS_FILE_SINK
	BOOST_AUTO_TEST_CASE(file_system_watcher_change_on_write)
	{
		ventura::absolute_path const watched_dir = ventura::get_current_working_directory(Si::throw_);
		ventura::absolute_path const test_file = watched_dir / "test.txt";
		
		boost::filesystem::create_directories(watched_dir.to_boost_path());
		Si::file_handle file = ventura::overwrite_file(Si::native_path_string(test_file.c_str())).move_value();

		test_single_event(
			watched_dir,
			[&file]
		{
			ventura::file_sink sink(file.handle);
			Si::throw_if_error(Si::append(sink, ventura::file_sink_element(Si::make_c_str_range("hello\n"))));
			Si::throw_if_error(Si::append(sink, ventura::file_sink_element(ventura::flush())));
			//we write to the file without closing it
		},
			[&test_file, &watched_dir](ventura::file_notification const &event)
		{
			BOOST_CHECK(event.type == ventura::file_notification_type::change_content || event.type == ventura::file_notification_type::change_content_or_metadata);
			BOOST_CHECK_EQUAL(test_file, watched_dir / event.name);
		});
	}
#endif

	BOOST_AUTO_TEST_CASE(file_system_watcher_change_on_close)
	{
		ventura::absolute_path const watched_dir = ventura::get_current_working_directory(Si::throw_);
		ventura::absolute_path const test_file = watched_dir / "test.txt";

		touch(test_file);

		std::ofstream file(test_file.c_str());
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
			[&test_file, &watched_dir](ventura::file_notification const &event)
		{
			BOOST_CHECK(event.type == ventura::file_notification_type::change_content || event.type == ventura::file_notification_type::change_content_or_metadata);
			BOOST_CHECK_EQUAL(test_file, watched_dir / event.name);
		});
	}

	BOOST_AUTO_TEST_CASE(file_system_watcher_change_metadata)
	{
		ventura::absolute_path const watched_dir = ventura::get_current_working_directory(Si::throw_);
		ventura::absolute_path const test_file = watched_dir / "test.txt";

		touch(test_file);

#ifdef _WIN32
		BOOST_REQUIRE(SetFileAttributesW(test_file.c_str(), FILE_ATTRIBUTE_NORMAL));
#else
		BOOST_REQUIRE(!chmod(test_file.c_str(), 0555));
#endif

		test_single_event(
			watched_dir,
			[&test_file]
		{
#ifdef _WIN32
			BOOST_REQUIRE(SetFileAttributesW(test_file.c_str(), FILE_ATTRIBUTE_TEMPORARY));
#else
			BOOST_REQUIRE(!chmod(test_file.c_str(), 0755));
#endif
		},
			[&test_file, &watched_dir](ventura::file_notification const &event)
		{
			BOOST_CHECK(ventura::file_notification_type::change_content_or_metadata == event.type || ventura::file_notification_type::change_metadata == event.type);
			BOOST_CHECK_EQUAL(test_file, watched_dir / event.name);
		});
	}

	BOOST_AUTO_TEST_CASE(file_system_watcher_move_out)
	{
		ventura::absolute_path const root_dir = ventura::get_current_working_directory(Si::throw_);
		ventura::absolute_path const watched_dir = root_dir / "watched";
		ventura::absolute_path const original_name = watched_dir / "test.txt";
		ventura::absolute_path const new_name = root_dir / "renamed.txt";

		boost::filesystem::remove_all(new_name.to_boost_path());
		boost::filesystem::remove_all(watched_dir.to_boost_path());

		boost::filesystem::create_directories(watched_dir.to_boost_path());
		touch(original_name);

		test_single_event(
			watched_dir,
			[&original_name, &new_name]
		{
			boost::filesystem::rename(original_name.to_boost_path(), new_name.to_boost_path());
		},
			[&original_name, &watched_dir](ventura::file_notification const &event)
		{
			BOOST_CHECK(ventura::file_notification_type::remove == event.type);
			BOOST_CHECK_EQUAL(original_name, watched_dir / event.name);
		});
	}

	BOOST_AUTO_TEST_CASE(file_system_watcher_move_in)
	{
		ventura::absolute_path const root_dir = ventura::get_current_working_directory(Si::throw_);
		ventura::absolute_path const watched_dir = root_dir / "watched";
		ventura::absolute_path const original_name = root_dir / "test.txt";
		ventura::absolute_path const new_name = watched_dir / "renamed.txt";

		boost::filesystem::remove_all(watched_dir.to_boost_path());

		boost::filesystem::create_directories(watched_dir.to_boost_path());
		touch(original_name);

		test_single_event(
			watched_dir,
			[&original_name, &new_name]
		{
			boost::filesystem::rename(original_name.to_boost_path(), new_name.to_boost_path());
		},
			[&new_name, &watched_dir](ventura::file_notification const &event)
		{
			BOOST_CHECK(ventura::file_notification_type::add == event.type);
			BOOST_CHECK_EQUAL(new_name, watched_dir / event.name);
		});
	}

#ifndef _WIN32
	//we cannot yet detect the renaming of the watched directory
	BOOST_AUTO_TEST_CASE(file_system_watcher_rename_watched_dir)
	{
		ventura::absolute_path const root_dir = ventura::get_current_working_directory(Si::throw_);
		ventura::absolute_path const watched_dir = root_dir / "watched";
		ventura::absolute_path const new_name = root_dir / "renamed";

		boost::filesystem::remove_all(watched_dir.to_boost_path());
		boost::filesystem::remove_all(new_name.to_boost_path());

		boost::filesystem::create_directories(watched_dir.to_boost_path());

		test_single_event(
			watched_dir,
			[&watched_dir, &new_name]
		{
			boost::filesystem::rename(watched_dir.to_boost_path(), new_name.to_boost_path());
		},
			[&watched_dir](ventura::file_notification const &event)
		{
			BOOST_CHECK(ventura::file_notification_type::move_self == event.type);
			BOOST_CHECK_EQUAL("", event.name.underlying());
		});
	}
#endif

#if !defined(_WIN32) && !defined(SILICIUM_TESTS_RUNNING_ON_TRAVIS_CI)
	//It is a known problem that travis-ci.org does not properly implement IN_DELETE_SELF,
	//so this test is pointless there.
	//https://github.com/travis-ci/travis-ci/issues/2342
	BOOST_AUTO_TEST_CASE(file_system_watcher_remove_watched_dir)
	{
		ventura::absolute_path const root_dir = ventura::get_current_working_directory(Si::throw_);
		ventura::absolute_path const watched_dir = root_dir / "watched";

		boost::filesystem::remove_all(watched_dir.to_boost_path());

		boost::filesystem::create_directories(watched_dir.to_boost_path());

		test_single_event(
			watched_dir,
			[&watched_dir]
		{
			boost::filesystem::remove(watched_dir.to_boost_path());
		},
			[&watched_dir](ventura::file_notification const &event)
		{
			BOOST_CHECK(ventura::file_notification_type::remove_self == event.type);
			BOOST_CHECK_EQUAL("", event.name.underlying());
		});
	}
#endif

#if SILICIUM_HAS_SPAWN_COROUTINE
	BOOST_AUTO_TEST_CASE(file_system_watcher_rename_in_dir)
	{
		ventura::absolute_path const watched_dir = ventura::get_current_working_directory(Si::throw_);
		ventura::absolute_path const test_file_a = watched_dir / "test_a.txt";
		ventura::absolute_path const test_file_b = watched_dir / "test_b.txt";

		touch(test_file_a);
		boost::filesystem::remove(test_file_b.to_boost_path());

		boost::asio::io_service io;
		ventura::single_directory_watcher watcher(io, watched_dir);

		bool got_something = false;

		spawn_coroutine([&watcher, &test_file_a, &test_file_b, &got_something](spawn_context &yield)
		{
			optional<error_or<ventura::file_notification>> first_event = yield.get_one(Si::ref(watcher));
			BOOST_REQUIRE(first_event);
			optional<error_or<ventura::file_notification>> second_event = yield.get_one(Si::ref(watcher));
			BOOST_REQUIRE(second_event);

			got_something = true;

			ventura::file_notification *remove = &first_event->get();
			ventura::file_notification *add = &second_event->get();
			if (add->type != ventura::file_notification_type::add)
			{
				using std::swap;
				swap(remove, add);
			}

			BOOST_CHECK(ventura::file_notification_type::remove == remove->type);
			BOOST_CHECK_EQUAL(leaf(test_file_a), remove->name);

			BOOST_CHECK(ventura::file_notification_type::add == add->type);
			BOOST_CHECK_EQUAL(leaf(test_file_b), add->name);
		});

		boost::filesystem::rename(test_file_a.to_boost_path(), test_file_b.to_boost_path());

		boost::asio::basic_waitable_timer<Si::steady_clock_if_available> timeout(io);
		timeout.expires_from_now(Si::chrono::seconds(1));
		timeout.async_wait([&io](boost::system::error_code)
		{
			io.stop();
		});

		io.run();

		BOOST_CHECK(got_something);
	}
#endif
#endif
}
