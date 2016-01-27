#include <silicium/sqlite3.hpp>
#include <boost/test/unit_test.hpp>

#if SILICIUM_HAS_EXCEPTIONS
#include <boost/filesystem/operations.hpp>

BOOST_AUTO_TEST_CASE(sqlite_open_or_create_file)
{
	boost::filesystem::path const test_file = boost::filesystem::current_path() / "test_sqlite_open_file.sqlite3";
	boost::filesystem::remove(test_file);
	{
		Si::error_or<Si::SQLite3::database_handle> database =
		    Si::SQLite3::open_or_create(Si::c_string(test_file.string().c_str()));
		BOOST_REQUIRE(!database.is_error());
		BOOST_CHECK(database.get());
		Si::SQLite3::database_handle moved = database.move_value();
		BOOST_CHECK(moved);
	}
	// reopen an existing file:
	{
		Si::error_or<Si::SQLite3::database_handle> database =
		    Si::SQLite3::open_or_create(Si::c_string(test_file.string().c_str()));
		BOOST_REQUIRE(!database.is_error());
		BOOST_CHECK(database.get());
		Si::SQLite3::database_handle moved = database.move_value();
		BOOST_CHECK(moved);
	}
}
#endif

BOOST_AUTO_TEST_CASE(sqlite_open_existing_memory)
{
	Si::error_or<Si::SQLite3::database_handle> database = Si::SQLite3::open_existing(":memory:");
	BOOST_REQUIRE(!database.is_error());
	BOOST_CHECK(database.get());
	Si::SQLite3::database_handle moved = database.move_value();
	BOOST_CHECK(moved);
}

BOOST_AUTO_TEST_CASE(sqlite_open_or_create_memory)
{
	Si::error_or<Si::SQLite3::database_handle> database = Si::SQLite3::open_or_create(":memory:");
	BOOST_REQUIRE(!database.is_error());
	BOOST_CHECK(database.get());
	Si::SQLite3::database_handle moved = database.move_value();
	BOOST_CHECK(moved);
}

BOOST_AUTO_TEST_CASE(sqlite_open_error)
{
	Si::error_or<Si::SQLite3::database_handle> database = Si::SQLite3::open_existing("/");
	BOOST_CHECK(database.is_error());
}

BOOST_AUTO_TEST_CASE(sqlite_prepare)
{
	Si::SQLite3::database_handle database = Si::SQLite3::open_existing(":memory:").move_value();
	Si::error_or<Si::SQLite3::statement_handle> statement = Si::SQLite3::prepare(*database, "SELECT 1");
	BOOST_REQUIRE(!statement.is_error());
	BOOST_CHECK(statement.get());
	Si::SQLite3::statement_handle moved = statement.move_value();
	BOOST_CHECK(moved);
}

BOOST_AUTO_TEST_CASE(sqlite_bind_int64)
{
	Si::SQLite3::database_handle database = Si::SQLite3::open_existing(":memory:").move_value();
	Si::SQLite3::statement_handle statement = Si::SQLite3::prepare(*database, "SELECT ?").move_value();
	BOOST_CHECK(!Si::SQLite3::bind(*statement, Si::SQLite3::positive_int::literal<1>(), 123456));
}

BOOST_AUTO_TEST_CASE(sqlite_step)
{
	Si::SQLite3::database_handle database = Si::SQLite3::open_existing(":memory:").move_value();
	Si::SQLite3::statement_handle statement = Si::SQLite3::prepare(*database, "SELECT 1").move_value();
	BOOST_REQUIRE_EQUAL(Si::SQLite3::step_result::row, Si::SQLite3::step(*statement).get());
	BOOST_REQUIRE_EQUAL(Si::SQLite3::positive_int::literal<1>(), Si::SQLite3::column_count(*statement));
	BOOST_CHECK_EQUAL(1, Si::SQLite3::column_int64(*statement, Si::SQLite3::positive_int::literal<0>()));
	BOOST_CHECK_EQUAL(Si::SQLite3::step_result::done, Si::SQLite3::step(*statement).get());
}

BOOST_AUTO_TEST_CASE(sqlite_column_double)
{
	Si::SQLite3::database_handle database = Si::SQLite3::open_existing(":memory:").move_value();
	Si::SQLite3::statement_handle statement = Si::SQLite3::prepare(*database, "SELECT 1.5").move_value();
	BOOST_REQUIRE_EQUAL(Si::SQLite3::step_result::row, Si::SQLite3::step(*statement).get());
	BOOST_REQUIRE_EQUAL(Si::SQLite3::positive_int::literal<1>(), Si::SQLite3::column_count(*statement));
	BOOST_CHECK_EQUAL(1.5, Si::SQLite3::column_double(*statement, Si::SQLite3::positive_int::literal<0>()));
}

BOOST_AUTO_TEST_CASE(sqlite_column_text)
{
	Si::SQLite3::database_handle database = Si::SQLite3::open_existing(":memory:").move_value();
	Si::SQLite3::statement_handle statement = Si::SQLite3::prepare(*database, "SELECT \"abc\"").move_value();
	BOOST_REQUIRE_EQUAL(Si::SQLite3::step_result::row, Si::SQLite3::step(*statement).get());
	BOOST_REQUIRE_EQUAL(Si::SQLite3::positive_int::literal<1>(), Si::SQLite3::column_count(*statement));
	Si::memory_range const expected = Si::make_c_str_range("abc");
	Si::memory_range const got = Si::SQLite3::column_text(*statement, Si::SQLite3::positive_int::literal<0>());
	BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), got.begin(), got.end());
}
