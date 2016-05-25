/**
 * @file
 * OpenM++ data library: db connection wrapper for SQLite
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef DB_EXEC_SQLITE_H
#define DB_EXEC_SQLITE_H

#include "dbExec.h"
#include "dbExecBase.h"

#include "sqlite3.h"
#include "extension_functions.h"

namespace openm
{
    /** db connection wrapper for SQLite. */
    class DbExecSqlite :  public DbExecBase, public IDbExec
    {
    public:
        /** create new db-connection. */
        DbExecSqlite(const string & i_connectionStr);

        /** close db-connection and cleanup connection resources. */
        ~DbExecSqlite(void) throw();

        /** select integer value of first (row,column) or default if no rows or value IS NULL. */
        int selectToInt(const string & i_sql, int i_default) override;

        /** select long value of first (row,column) or default if no rows or value IS NULL. */
        long long selectToLong(const string & i_sql, long long i_default) override;

        /** select string value of first (row,column) or empty "" string if no rows or value IS NULL. */
        string selectToStr(const string & i_sql) override;

        /** select boolean value of first (row,column) or false if no rows or value IS NULL. */
        bool selectToBool(const string & i_sql) override;

        /** select double value of first (row,column) or default if no rows or value IS NULL. */
        double selectToDouble(const string & i_sql, double i_default) override;

        /** select string vector of first row or empty vector if no rows, empty "" string used for NULLs. */
        vector<string> selectRowStr(const string & i_sql) override;

        /** select vector of rows, each row created and field values set by row adapter. */
        IRowBaseVec selectRowVector(const string & i_sql, const IRowAdapter & i_adapter) override;

        /** select list of rows, each row created and field values set by row adapter. */
        IRowBaseList selectRowList(const string & i_sql, const IRowAdapter & i_adapter) override;

        /** select and process rows: each row created by row adapter and passed to processor. */
        void selectToRowProcessor(const string & i_sql, const IRowAdapter & i_adapter, IRowProcessor & i_processor) override;

        /** select column into io_valueArray[i_size] buffer and return row count. */
        size_t selectColumn(
            const string & i_sql, int i_column, const type_info & i_type, size_t i_size, void * io_valueArr
        ) override;

        /** execute sql statement (update, insert, delete, create, etc). */
        size_t update(const string & i_sql) override;

        /** begin transaction, throw exception if transaction already active or statement is active. */
        void beginTransaction(void) override;

        /** begin transaction in multi-threaded environment, throw exception if transaction already active or statement is active. */
        unique_lock<recursive_mutex> beginTransactionThreaded(void) override;

        /** commit transaction, does nothing if no active transaction, throw exception if statement is active. */
        void commit(void) override;

        /** rollback transaction, does nothing if no active transaction, throw exception if statement is active. */
        void rollback(void) override;

        /** return true in transaction scope. */
        bool isTransaction(void) override { return DbExecBase::isTransaction(); }

        /** create new statement with specified parameters. */
        void createStatement(const string & i_sql, int i_paramCount, const type_info ** i_typeArr) override;

        /** release statement resources. */
        void releaseStatement(void) throw() override;

        /** execute statement with parameters. */
        void executeStatement(int i_paramCount, const DbValue * i_valueArr) override;

    private:
        sqlite3 * theDb;                    // sqlite db
        sqlite3_stmt * theStmt;             // sqlite statement

        // method to bind parameter value
        typedef void(DbExecSqlite::*BindHandler)(int, const DbValue &);

        vector<BindHandler> bindFncVec;     // methods to bind parameter values to the statement

        /** close db-connection and cleanup connection resources. */
        void cleanup(void) throw() override;

        /** validate connection properties. */
        void validateConnectionProps(void) override;

        // select value of first (row,column) or default if no records or value is NULL
        template <typename TCvt>
        TCvt selectTo(const string & i_sql, const TCvt & i_default, TCvt (DbExecSqlite::*ToRetType)(int));

        // retrieve single column field values into io_valueArray[i_size] buffer and return row count
        template <typename TCol>
        size_t retrieveColumnTo(int i_column, size_t i_size, void* io_valueArr, TCol (DbExecSqlite::*ToRetType)(int));

        // bind integer sql parameter at specified position
        void bindLong(int i_position, const DbValue & i_value);

        // bind double sql parameter at specified position, use NULL if double value not is finite
        template<typename TDbl>
        void bindDbl(int i_position, const DbValue & i_value);

        // bind bool sql parameter at specified position, use integer one if value is true else use zero
        void bindBool(int i_position, const DbValue & i_value);

        // bind string sql parameter at specified position, length of source string expected to be less than OM_STRLEN_MAX
        void bindStr(int i_position, const DbValue & i_value);

        // convert field values of current row
        template<typename TInt> TInt fieldToInt(int i_column);
        template<typename TDbl> TDbl fieldToDbl(int i_column);
        string fieldToStr(int i_column);
        int64_t fieldToInt64(int i_column);
        double fieldToDouble(int i_column);
        bool fieldToBool(int i_column);

        // set integer field value by using row adapter
        template<typename TInt>  
        void setFieldInt(IRowBase * i_row, int i_column, const IRowAdapter & i_adapter);

        // set double field value by using row adapter
        template<typename TDbl>  
        void setFieldDbl(IRowBase * i_row, int i_column, const IRowAdapter & i_adapter);

        // set boolean field value by using row adapter
        void setFieldBool(IRowBase * i_row, int i_column, const IRowAdapter & i_adapter);

        // set string field value by using row adapter
        void setFieldStr(IRowBase * i_row, int i_column, const IRowAdapter & i_adapter);

    private:
        DbExecSqlite(const DbExecSqlite & i_dbExec) = delete;
        DbExecSqlite & operator=(const DbExecSqlite & i_dbExec) = delete;
    };

    // setter to pass field value into row adapter
    typedef void (DbExecSqlite::*SetFieldHandler)(IRowBase *, int, const IRowAdapter &);
}

#endif  // DB_EXEC_SQLITE_H
