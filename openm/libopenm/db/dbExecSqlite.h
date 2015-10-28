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
        int selectToInt(const string & i_sql, int i_default);

        /** select long value of first (row,column) or default if no rows or value IS NULL. */
        long long selectToLong(const string & i_sql, long long i_default);

        /** select string value of first (row,column) or empty "" string if no rows or value IS NULL. */
        string selectToStr(const string & i_sql);

        /** select boolean value of first (row,column) or false if no rows or value IS NULL. */
        bool selectToBool(const string & i_sql);

        /** select double value of first (row,column) or default if no rows or value IS NULL. */
        double selectToDouble(const string & i_sql, double i_default);

        /** select string vector of first row or empty vector if no rows, empty "" string used for NULLs. */
        vector<string> selectRowStr(const string & i_sql);

        /** select vector of rows, each row created and field values set by row adapter. */
        IRowBaseVec selectRowList(const string & i_sql, const IRowAdapter & i_adapter);

        /** select column into io_valueArray[i_size] buffer and return row count. */
        long long selectColumn(const string & i_sql, int i_column, const type_info & i_type, long long i_size, void * io_valueArr);

        /** execute sql statement (update, insert, delete, create, etc). */
        long long update(const string & i_sql);

        /** begin transaction, throw exception if transaction already active or statement is active. */
        void beginTransaction(void);

        /** begin transaction in multi-threaded environment, throw exception if transaction already active or statement is active. */
        unique_lock<recursive_mutex> beginTransactionThreaded(void);

        /** commit transaction, does nothing if no active transaction, throw exception if statement is active. */
        void commit(void);

        /** rollback transaction, does nothing if no active transaction, throw exception if statement is active. */
        void rollback(void);

        /** return true in transaction scope. */
        bool isTransaction(void) { return DbExecBase::isTransaction(); }

        /** create new statement with specified parameters. */
        void createStatement(const string & i_sql, int i_paramCount, const type_info ** i_typeArr);

        /** release statement resources. */
        void releaseStatement(void) throw();

        /** execute statement with parameters. */
        void executeStatement(int i_paramCount, const DbValue * i_valueArr);

    private:
        sqlite3 * theDb;        // sqlite db
        sqlite3_stmt * theStmt; // sqlite statement

        // method to bind parameter value
        typedef void(DbExecSqlite::*BindHandler)(int, const DbValue &);

        vector<BindHandler> bindFncVec;     // methods to bind parameter values to the statement

        /** close db-connection and cleanup connection resources. */
        void cleanup(void) throw();

        /** validate connection properties. */
        void validateConnectionProps(void);

        // select value of first (row,column) or default if no records or value is NULL
        template <typename TCvt>
        TCvt selectTo(const string & i_sql, const TCvt & i_default, TCvt (DbExecSqlite::*ToRetType)(int));

        // retrieve single column field values into io_valueArray[i_size] buffer and return row count
        template <typename TCol>
        long long retrieveColumnTo(int i_column, long long i_size, void* io_valueArr, TCol (DbExecSqlite::*ToRetType)(int));

        // bind integer sql parameter at specified position
        void bindLong(int i_position, const DbValue & i_value);

        // bind double sql parameter at specified position, use NULL if double value not is finite
        void bindDbl(int i_position, const DbValue & i_value);

        // bind bool sql parameter at specified position, use integer one if value is true else use zero
        void bindBool(int i_position, const DbValue & i_value);

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
