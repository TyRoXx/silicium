#ifndef SILICIUM_SQLITE3_HPP
#define SILICIUM_SQLITE3_HPP

#include <silicium/c_string.hpp>
#include <silicium/error_or.hpp>
#include <sqlite3.h>
#include <memory>

namespace Si
{
	namespace SQLite3
	{
		struct database_deleter
		{
			void operator()(sqlite3 *database) const
			{
				assert(database);
				sqlite3_close(database);
			}
		};

		typedef std::unique_ptr<sqlite3, database_deleter> database_handle;

		struct statement_deleter
		{
			void operator()(sqlite3_stmt *statement) const
			{
				assert(statement);
				sqlite3_finalize(statement);
			}
		};

		typedef std::unique_ptr<sqlite3_stmt, statement_deleter> statement_handle;

		struct error_category : boost::system::error_category
		{
			virtual const char *name() const BOOST_SYSTEM_NOEXCEPT SILICIUM_OVERRIDE
			{
				return "sqlite3";
			}

	        virtual std::string message(int ev) const SILICIUM_OVERRIDE
			{
#if SQLITE_VERSION_NUMBER > 3007009
				return sqlite3_errstr(ev);
#else
				(void)ev;
				return "?";
#endif
			}
		};

		inline boost::system::error_category &sqlite_category()
		{
			static error_category instance;
			return instance;
		}

		inline boost::system::error_code make_error_code(int code)
		{
			return boost::system::error_code(code, sqlite_category());
		}

		inline error_or<database_handle> open(c_string path)
		{
			sqlite3 *database;
			int rc = sqlite3_open_v2(path.c_str(), &database, SQLITE_OPEN_READWRITE, nullptr);
			if (rc != SQLITE_OK)
			{
				return make_error_code(rc);
			}
			return database_handle(database);
		}

		inline error_or<statement_handle> prepare(sqlite3 &database, c_string query)
		{
			sqlite3_stmt *statement;
			int rc = sqlite3_prepare_v2(&database, query.c_str(), -1, &statement, nullptr);
			if (rc != SQLITE_OK)
			{
				return make_error_code(rc);
			}
			return statement_handle(statement);
		}

		inline boost::system::error_code bind(sqlite3_stmt &statement, int zero_based_index, sqlite3_int64 value)
		{
			return make_error_code(sqlite3_bind_int64(&statement, zero_based_index, value));
		}

		inline boost::system::error_code bind(sqlite3_stmt &statement, int zero_based_index, char const *begin, int length)
		{
			return make_error_code(sqlite3_bind_text(&statement, zero_based_index, begin, length, nullptr));
		}
	}
}

#endif
