/**
 * @file
 * OpenM++ data library: public interface
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef DB_EXEC_H
#define DB_EXEC_H

#include <mutex>
using namespace std;

#include "libopenm/omLog.h"
#include "libopenm/db/dbCommon.h"

namespace openm
{
    /** database connection wrapper to execute sql commands. */
    class IDbExec
    {
    public:
        /** close db-connection and cleanup connection resources. */
        virtual ~IDbExec(void) noexcept = 0;

        /** db-connection factory: create new db-connection.
         *
         * @param[in] i_sqlProvider   sql provider name, ie: SQLITE
         * @param[in] i_connectionStr db connection string
         */
        static IDbExec * create(const string & i_sqlProvider, const string & i_connectionStr);

        /**
         * select integer value of first (row,column) or default if no rows or value IS NULL.
         *
         * @param[in]   i_sql       select sql query
         * @param[in]   i_default   default value if no row(s) selected or db-value IS NULL
         *
         * @return  integer value of first (row,column) or default if no rows or value IS NULL
         */
        virtual int selectToInt(const string & i_sql, int i_default) = 0;

        /**
         * select long value of first (row,column) or default if no rows or value IS NULL.
         *
         * @param[in]   i_sql       select sql query
         * @param[in]   i_default   default value if no row(s) selected or db-value IS NULL
         *
         * @return  long value of first (row,column) or default if no rows or value IS NULL
         */
        virtual long long selectToLong(const string & i_sql, long long i_default) = 0;

        /**
         * select string value of first (row,column) or empty "" string if no rows or value IS NULL.
         *
         * @param[in]   i_sql       select sql query
         *
         * @return  string value of first (row,column) or empty "" string if no rows or value IS NULL
         */
        virtual string selectToStr(const string & i_sql) = 0;

        /**
         * select boolean value of first (row,column) or false if no rows or value IS NULL.
         *
         * @param[in]   i_sql       select sql query
         *
         * @return  boolean value of first (row,column) or false if no rows or value IS NULL
         */
        virtual bool selectToBool(const string & i_sql) = 0;

        /**
         * select double value of first (row,column) or default if no rows or value IS NULL.
         *
         * @param[in]   i_sql       select sql query
         * @param[in]   i_default   default value if no row(s) selected or db-value IS NULL
         *
         * @return  double value of first (row,column) or default if no rows or value IS NULL
         */
        virtual double selectToDouble(const string & i_sql, double i_default) = 0;

        /**
         * select string vector of first row or empty vector if no rows, empty "" string used for NULLs.
         *
         * @param[in]   i_sql       select sql query
         *
         * @return  string vector of first row or empty vector if no rows, empty "" string used for NULLs
         */
        virtual vector<string> selectRowStr(const string & i_sql) = 0;

        /**
         * select vector of rows, each row created and field values set by row adapter.
         *
         * @param[in]   i_sql       select sql query
         * @param[in]   i_adapter   row adapter class to create rows and set values
         *
         * @return  vector of db-rows
         */
        virtual IRowBaseVec selectRowVector(const string & i_sql, const IRowAdapter & i_adapter) = 0;

        /**
         * select list of rows, each row created and field values set by row adapter.
         *
         * @param[in]   i_sql       select sql query
         * @param[in]   i_adapter   row adapter class to create rows and set values
         *
         * @return  list of db-rows
         */
        virtual IRowBaseList selectRowList(const string & i_sql, const IRowAdapter & i_adapter) = 0;

        /**
         * select and process rows: each row created by row adapter and passed to processor.
         *
         * @param[in]   i_sql       select sql query
         * @param[in]   i_adapter   row adapter class to create rows and set values
         * @param[in]   i_processor interface to process each row, ie: calculate sum
         */
        virtual void selectToRowProcessor(const string & i_sql, const IRowAdapter & i_adapter, IRowProcessor & i_processor) = 0;

        /**
         * select column into io_valueArray[i_size] buffer and return row count.
         *
         * @param[in]      i_sql       select sql query
         * @param[in]      i_column    zero-based column index
         * @param[in]      i_type      type of io_valueArr array (target type of values)
         * @param[in]      i_size      size of io_valueArr array
         * @param[in,out]  io_valueArr one dimensional array to put selected values
         *
         * @return  row count, may throw exception if row count not equal i_size
         *
         * convert db-values into target type and put into io_valueArr \n
         * if db-value in current row IS NULL then do not change io_valueArray at this index \n
         * if target type is string then io_valueArray expected to be string[] \n
         * it may throw exception if selected row count not equal i_size
         */
        virtual size_t selectColumn(
            const string & i_sql, int i_column, const type_info & i_type, size_t i_size, void * io_valueArr
        ) = 0;

        /**
         * execute sql statement (update, insert, delete, create, etc).
         *
         * @param[in] i_sql     sql statement of "update" type (update, insert, delete, create, etc.)
         *
         * @return   number of affected rows (db-provider specific).
         */
        virtual size_t update(const string & i_sql) = 0;

        /** begin transaction. */
        virtual void beginTransaction(void) = 0;

        /** begin transaction in multi-threaded environment. */
        virtual unique_lock<recursive_mutex> beginTransactionThreaded(void) = 0;

        /** commit transaction, does nothing if no active transaction. */
        virtual void commit(void) = 0;

        /** rollback transaction. */
        virtual void rollback(void) = 0;

        /** return true in transaction scope. */
        virtual bool isTransaction(void) = 0;

        /**
         * create new statement with specified parameters.
         *
         * @param[in] i_sql        sql to create statement
         * @param[in] i_paramCount number of parameters
         * @param[in] i_typeArr    array of parameters type, use char * for VARCHAR
         *
         * usage example: \n
         * @code
         *      createStatement();
         *      while(...) {
         *          for (int k = 0; k < paramCount; k++) {
         *              valueArr[k] = some value;
         *          }
         *          executeStatement(paramCount, valueArr);
         *      }
         *      releaseStatement();
         * @endcode
         */
        virtual void createStatement(const string & i_sql, int i_paramCount, const type_info ** i_typeArr) = 0;

        /**
         * release statement resources.
         *
         * usage example: \n
         * @code
         *      createStatement();
         *      while(...) {
         *          for (int k = 0; k < paramCount; k++) {
         *              valueArr[k] = some value;
         *          }
         *          executeStatement(paramCount, valueArr);
         *      }
         *      releaseStatement();
         * @endcode
         */
        virtual void releaseStatement(void) noexcept = 0;

        /**
         * execute statement with parameters.
         *
         * @param[in] i_paramCount number of parameters
         * @param[in] i_valueArr   array of parameters value, use char * for VARCHAR
         *
         * this method can be used only after createStatement() call \n
         * usage example: \n
         * @code
         *      createStatement();
         *      while(...) {
         *          for (int k = 0; k < paramCount; k++) {
         *              valueArr[k] = some value;
         *          }
         *          executeStatement(paramCount, valueArr);
         *      }
         *      releaseStatement();
         * @endcode
         */
        virtual void executeStatement(int i_paramCount, const DbValue * i_valueArr) = 0;

        /** return list of provider names from supplied comma or semicolon separated string or exception on invalid name. */
        static list<string> parseListOfProviderNames(const string & i_sqlProviderNames);

        /** check if provider name is valid. */
        static bool isValidProviderName(const char * i_sqlProvider);

        /** retrun true if i_keword matched one of SQL reserved keywords, comparison is case neutral and limited by i_length chars */
        static bool isSqlKeyword(const char * i_keyword, size_t i_length = 0);

        /** return max length of db table or view name. */
        static const int maxDbTableNameSize;

        /** return type name for BIGINT sql type */
        static string bigIntTypeName(const string & i_sqlProvider);

        /** return type name for FLOAT standard sql type */
        static string floatTypeName(const string & i_sqlProvider);

        /** return column type DDL for long VARCHAR columns, use it for len > 255. */
        static string textTypeName(const string & i_sqlProvider, int i_size);

        /** return sql statement to begin transaction (db-provider specific). */
        static string makeSqlBeginTransaction(const string & i_sqlProvider);

        /** return sql statement to commit transaction (db-provider specific). */
        static string makeSqlCommitTransaction(const string & i_sqlProvider);

        /**
         * make sql statement to create table if not exists.
         *
         * @param[in] i_sqlProvider   sql provider name, ie: SQLITE
         * @param[in] i_tableName     table name to create
         * @param[in] i_tableBodySql  table body definition sql: columns, keys, etc.
         *
         * @return  string with create table statment (db-provider specific)
         *
         * it does return db-provider specific sql to create table if not already exists, for example: \n
         * @code
         *     CREATE TABLE IF NOT EXISTS tableName (...column definition and other table body sql...)
         * @endcode
         */
        static string makeSqlCreateTableIfNotExist(
            const string & i_sqlProvider, const string & i_tableName, const string & i_tableBodySql
        );

        /**
         * make sql statement to create or replace view.
         *
         * @param[in] i_sqlProvider   sql provider name, ie: SQLITE
         * @param[in] i_viewName      view name to create
         * @param[in] i_viewBodySql   view body definition sql
         *
         * @return  string with create view statment (db-provider specific)
         *
         * it does return db-provider specific sql to create view if not already exists, for example: \n
         * @code
         *      CREATE VIEW IF NOT EXISTS viewName AS ...select or other view body deifinition sql...
         * or:
         *      CREATE OR REPLACE VIEW viewName AS ...select or other view body deifinition sql...
         * @endcode
         */
        static string makeSqlCreateViewReplace(
            const string & i_sqlProvider, const string & i_viewName, const string & i_viewBodySql
        );

        /** parse and execute list of sql statements.
        *
        * @param[in] i_sqlScript    sql statements separated by ; semicolons
        *
        * source text may contain multiple sql statements separated by ; semicolons.
        * it can be multiple line statement or multiple sql statements; on single line.
        * it can include 'sql''quoted const with -- or ; semicolon' inside. \n
        *
        * end of line <cr><lf> replaced by space.
        * end of statement ; semicolons and --comments removed. \n
        *
        * source string must be utf-8 or "code page" encoded, utf-16 or utf-32 NOT supported.
        *
        * @code
        *    DELETE FROM tbl WHERE code = 1234;  -- comment after sql
        *    -- comment only line
        *    INSERT INTO tbl
        *      (code, label)  -- comment inside of sql
        *    VALUES
        *      (4567,
        *      'label --> not a comment because inside of ''quote
        *      continue ''string to the new line')
        *    ; UPDATE tbl SET label = '' WHERE code = 4567; -- multiple sqls ; on single line
        * @endcode
        */
        virtual void runSqlScript(const string & i_sqlScript) = 0;
    };
}

#endif  // DB_EXEC_H
