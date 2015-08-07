#include <silicium/single_directory_watcher.hpp>
#include <silicium/observable/consume.hpp>
#include <silicium/observable/spawn_coroutine.hpp>
#include <silicium/open.hpp>
#include <silicium/sink/file_sink.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <fstream>
#include <chrono>
#if SILICIUM_HAS_SINGLE_DIRECTORY_WATCHER
#	include <boost/filesystem/operations.hpp>
#endif

namespace Si
{
#if SILICIUM_HAS_SINGLE_DIRECTORY_WATCHER
	namespace
	{
		void touch(Si::absolute_path const &name)
		{
			//remove existing file to reset the attributes
			Si::remove_file(name);

			std::ofstream file(name.c_str());
			BOOST_REQUIRE(file);
		}

		void test_single_event(
			Si::absolute_path const &watched_dir,
			std::function<void ()> const &provoke_event,
			std::function<void (file_notification const &)> const &on_event)
		{
			boost::asio::io_service io;
			single_directory_watcher watcher(io, watched_dir);

			bool got_event = false;
			auto consumer = consume<error_or<file_notification>>([&io, &got_event, &on_event](error_or<file_notification> const &event)
			{
				io.stop();
				on_event(event.get());
				got_event = true;
			});
			watcher.async_get_one(Si::observe_by_ref(consumer));

			provoke_event();

			boost::asio::basic_waitable_timer<std::chrono::steady_clock> timeout(io);
			timeout.expires_from_now(std::chrono::seconds(1));
			timeout.async_wait([&io](boost::system::error_code)
			{
				io.stop();
			});

			io.run();

			BOOST_CHECK(got_event);
		}
	}

	BOOST_AUTO_TEST_CASE(file_system_watcher_add_regular)
	{
		Si::absolute_path const watched_dir = Si::get_current_working_directory();
		Si::absolute_path const test_file = watched_dir / Si::relative_path("test.txt");
		Si::remove_file(test_file);

		test_single_event(
			watched_dir,
			std::bind(touch, test_file),
			[&test_file](file_notification const &event)
		{
			BOOST_CHECK(file_notification_type::add == event.type);
			BOOST_CHECK(equivalent(test_file.to_boost_path(), event.name.to_boost_path()));
		});
	}

	BOOST_AUTO_TEST_CASE(file_system_watcher_add_directory)
	{
		Si::absolute_path const watched_dir = Si::get_current_working_directory();
		Si::absolute_path const test_file = watched_dir / "test";
		Si::remove_file(test_file);

		test_single_event(
			watched_dir,
			[&test_file]
		{
			boost::filesystem::create_directory(test_file.to_boost_path());
		},
			[&test_file](file_notification const &event)
		{
			BOOST_CHECK(file_notification_type::add == event.type);
			BOOST_CHECK(equivalent(test_file.to_boost_path(), event.name.to_boost_path()));
		});
	}

	BOOST_AUTO_TEST_CASE(file_system_watcher_remove)
	{
		Si::absolute_path const watched_dir = Si::get_current_working_directory();
		Si::absolute_path const test_file = watched_dir / "test.txt";

		touch(test_file);

		test_single_event(
			watched_dir,
			[&test_file]
		{
			boost::filesystem::remove(test_file.to_boost_path());
		},
			[&test_file, &watched_dir](file_notification const &event)
		{
			BOOST_CHECK(file_notification_type::remove == event.type);
			BOOST_CHECK_EQUAL(test_file, watched_dir / event.name);
		});
	}

	BOOST_AUTO_TEST_CASE(file_system_watcher_remove_directory)
	{
		Si::absolute_path const watched_dir = Si::get_current_working_directory();
		Si::absolute_path const test_file = watched_dir / "test";

		boost::filesystem::create_directory(test_file.to_boost_path());

		test_single_event(
			watched_dir,
			[&test_file]
		{
			boost::filesystem::remove(test_file.to_boost_path());
		},
			[&test_file, &watched_dir](file_notification const &event)
		{
			BOOST_CHECK(file_notification_type::remove == event.type);
			BOOST_CHECK_EQUAL(test_file, watched_dir / event.name);
		});
	}


#if SILICIUM_HAS_FILE_SINK
	BOOST_AUTO_TEST_CASE(file_system_watcher_change_on_write)
	{
		Si::absolute_path const watched_dir = Si::get_current_working_directory();
		Si::absolute_path const test_file = watched_dir / "test.txt";
		
		boost::filesystem::create_directories(watched_dir.to_boost_path());
		Si::file_handle file = Si::overwrite_file(Si::native_path_string(test_file.c_str())).move_value();

		test_single_event(
			watched_dir,
			[&file]
		{
			Si::file_sink sink(file.handle);
			Si::append(sink, Si::file_sink_element(Si::make_c_str_range("hello\n")));
			Si::append(sink, Si::file_sink_element(Si::flush()));
			//we write to the file without closing it
		},
			[&test_file, &watched_dir](file_notification const &event)
		{
			BOOST_CHECK(event.type == file_notification_type::change_content || event.type == file_notification_type::change_content_or_metadata);
			BOOST_CHECK_EQUAL(test_file, watched_dir / event.name);
		});
	}
#endif

	BOOST_AUTO_TEST_CASE(file_system_watcher_change_on_close)
	{
		Si::absolute_path const watched_dir = Si::get_current_working_directory();
		Si::absolute_path const test_file = watched_dir / "test.txt";

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
			[&test_file, &watched_dir](file_notification const &event)
		{
			BOOST_CHECK(event.type == file_notification_type::change_content || event.type == file_notification_type::change_content_or_metadata);
			BOOST_CHECK_EQUAL(test_file, watched_dir / event.name);
		});
	}

	BOOST_AUTO_TEST_CASE(file_system_watcher_change_metadata)
	{
		Si::absolute_path const watched_dir = Si::get_current_working_directory();
		Si::absolute_path const test_file = watched_dir / "test.txt";

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
			[&test_file, &watched_dir](file_notification const &event)
		{
			BOOST_CHECK(file_notification_type::change_content_or_metadata == event.type || file_notification_type::change_metadata == event.type);
			BOOST_CHECK_EQUAL(test_file, watched_dir / event.name);
		});
	}

	BOOST_AUTO_TEST_CASE(file_system_watcher_move_out)
	{
		Si::absolute_path const root_dir = Si::get_current_working_directory();
		Si::absolute_path const watched_dir = root_dir / "watched";
		Si::absolute_path const original_name = watched_dir / "test.txt";
		Si::absolute_path const new_name = root_dir / "renamed.txt";

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
			[&original_name, &watched_dir](file_notification const &event)
		{
			BOOST_CHECK(file_notification_type::remove == event.type);
			BOOST_CHECK_EQUAL(original_name, watched_dir / event.name);
		});
	}

	BOOST_AUTO_TEST_CASE(file_system_watcher_move_in)
	{
		Si::absolute_path const root_dir = Si::get_current_working_directory();
		Si::absolute_path const watched_dir = root_dir / "watched";
		Si::absolute_path const original_name = root_dir / "test.txt";
		Si::absolute_path const new_name = watched_dir / "renamed.txt";

		boost::filesystem::remove_all(watched_dir.to_boost_path());

		boost::filesystem::create_directories(watched_dir.to_boost_path());
		touch(original_name);

		test_single_event(
			watched_dir,
			[&original_name, &new_name]
		{
			boost::filesystem::rename(original_name.to_boost_path(), new_name.to_boost_path());
		},
			[&new_name, &watched_dir](file_notification const &event)
		{
			BOOST_CHECK(file_notification_type::add == event.type);
			BOOST_CHECK_EQUAL(new_name, watched_dir / event.name);
		});
	}

#ifndef _WIN32
	//we cannot yet detect the renaming of the watched directory
	BOOST_AUTO_TEST_CASE(file_system_watcher_rename_watched_dir)
	{
		Si::absolute_path const root_dir = Si::get_current_working_directory();
		Si::absolute_path const watched_dir = root_dir / "watched";
		Si::absolute_path const new_name = root_dir / "renamed";

		boost::filesystem::remove_all(watched_dir.to_boost_path());
		boost::filesystem::remove_all(new_name.to_boost_path());

		boost::filesystem::create_directories(watched_dir.to_boost_path());

		test_single_event(
			watched_dir,
			[&watched_dir, &new_name]
		{
			boost::filesystem::rename(watched_dir.to_boost_path(), new_name.to_boost_path());
		},
			[&watched_dir](file_notification const &event)
		{
			BOOST_CHECK(file_notification_type::move_self == event.type);
			BOOST_CHECK_EQUAL("", event.name.underlying());
		});
	}
#endif

#ifndef _WIN32
	//we cannot yet detect the removal of the watched directory
	BOOST_AUTO_TEST_CASE(file_system_watcher_remove_watched_dir)
	{
		Si::absolute_path const root_dir = Si::get_current_working_directory();
		Si::absolute_path const watched_dir = root_dir / "watched";

		boost::filesystem::remove_all(watched_dir.to_boost_path());

		boost::filesystem::create_directories(watched_dir.to_boost_path());

		test_single_event(
			watched_dir,
			[&watched_dir]
		{
			boost::filesystem::remove(watched_dir.to_boost_path());
		},
			[&watched_dir](file_notification const &event)
		{
			BOOST_CHECK(file_notification_type::remove_self == event.type);
			BOOST_CHECK_EQUAL("", event.name.underlying());
		});
	}
#endif

#if SILICIUM_HAS_SPAWN_COROUTINE
	BOOST_AUTO_TEST_CASE(file_system_watcher_rename_in_dir)
	{
		Si::absolute_path const watched_dir = Si::get_current_working_directory();
		Si::absolute_path const test_file_a = watched_dir / "test_a.txt";
		Si::absolute_path const test_file_b = watched_dir / "test_b.txt";

		touch(test_file_a);
		boost::filesystem::remove(test_file_b.to_boost_path());

		boost::asio::io_service io;
		single_directory_watcher watcher(io, watched_dir);

		bool got_something = false;

		spawn_coroutine([&watcher, &test_file_a, &test_file_b, &got_something](spawn_context &yield)
		{
			optional<error_or<file_notification>> first_event = yield.get_one(Si::ref(watcher));
			BOOST_REQUIRE(first_event);
			optional<error_or<file_notification>> second_event = yield.get_one(Si::ref(watcher));
			BOOST_REQUIRE(second_event);

			got_something = true;

			file_notification *remove = &first_event->get();
			file_notification *add = &second_event->get();
			if (add->type != file_notification_type::add)
			{
				using std::swap;
				swap(remove, add);
			}

			BOOST_CHECK(file_notification_type::remove == remove->type);
			BOOST_CHECK_EQUAL(leaf(test_file_a), remove->name);

			BOOST_CHECK(file_notification_type::add == add->type);
			BOOST_CHECK_EQUAL(leaf(test_file_b), add->name);
		});

		boost::filesystem::rename(test_file_a.to_boost_path(), test_file_b.to_boost_path());

		boost::asio::basic_waitable_timer<std::chrono::steady_clock> timeout(io);
		timeout.expires_from_now(std::chrono::seconds(1));
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
