/**
 * @file
 * OpenM++ data library: common classes for db connection wrapper
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef DB_EXEC_BASE_H
#define DB_EXEC_BASE_H

using namespace std;

#include "helper.h"
#include "dbExec.h"

namespace openm
{
    /** base class for database connection wrapper */
    class DbExecBase
    {
    public:
        /** prepare to open new db-connection. */
        DbExecBase(const string & i_connectionStr);

        /** cleanup connection resources. */
        ~DbExecBase(void) noexcept { }

        /** return true in transaction scope. */
        bool isTransaction(void);

    protected:
        /** if true then transaction is active */
        thread::id trxThreadId;

        /** connection properties as key+value string pairs (case neutral). */
        NoCaseMap connProps;

    protected:
        /** return true if other thread have active transaction. */
        bool isTransactionNonOwn(void);

        /** set transaction ownership status. */
        void setTransactionActive(void);

        /** release transaction: clean active transaction status. */
        void releaseTransaction(void);

        /** return string value of connection property by key (case neutral) or empty "" string if key not found. */
        string strConnProperty(const string & i_key) const;

        /** return true if value of connection property is 'true', 'yes', '1' (case neutral). */
        bool boolConnProperty(const string & i_key) const;

        /** return long value of connection property or default value if key not found (case neutral). */
        long long longConnProperty(const string & i_key, long i_default) const;

        /** parse and execute list of sql statements. */
        static void runSqlScript(IDbExec * i_dbExec, const string & i_sqlScript);

    private:
        /** parse connection string and return case-neutral map of key+value string pairs. */
        NoCaseMap parseConnectionStr(const string & i_connectionStr);

        // cleanup connection resources
        virtual void cleanup(void) noexcept = 0;

        // validate connection properties
        virtual void validateConnectionProps(void) = 0;

    private:
        DbExecBase(const DbExecBase & i_dbExec) = delete;
        DbExecBase & operator=(const DbExecBase & i_dbExec) = delete;
    };

    /** row processor to append rows to sequence container, ie: to list or vector */
    template<typename TContainer> 
    class RowContainerInserter : public IRowProcessor 
    {
    public:

        /** append row to sequence container, ie: to list or vector */
        void processRow(IRowBaseUptr & i_row) override { rowContainer.push_back(std::move(i_row)); };

        /** row container */
        TContainer rowContainer;
    };

    /** mutex to lock database operations */
    extern recursive_mutex dbMutex;
}

#endif  // DB_EXEC_BASE_H
