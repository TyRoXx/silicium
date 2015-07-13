#include <silicium/sqlite3.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(sqlite_open)
{
	Si::error_or<Si::SQLite3::database_handle> database = Si::SQLite3::open(":memory:");
	BOOST_REQUIRE(!database.is_error());
	BOOST_CHECK(database.get());
	Si::SQLite3::database_handle moved = database.move_value();
	BOOST_CHECK(moved);
}

BOOST_AUTO_TEST_CASE(sqlite_open_error)
{
	Si::error_or<Si::SQLite3::database_handle> database = Si::SQLite3::open("/");
	BOOST_CHECK(database.is_error());
}

BOOST_AUTO_TEST_CASE(sqlite_prepare)
{
	Si::SQLite3::database_handle database = Si::SQLite3::open(":memory:").move_value();
	Si::error_or<Si::SQLite3::statement_handle> statement = Si::SQLite3::prepare(*database, "SELECT 1");
	BOOST_REQUIRE(!statement.is_error());
	BOOST_CHECK(statement.get());
}
