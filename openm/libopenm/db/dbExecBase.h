/**
 * @file
 * OpenM++ data library: base class for db connection wrapper
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef DB_EXEC_BASE_H
#define DB_EXEC_BASE_H

using namespace std;

#include "dbExec.h"

namespace openm
{
    /** base class for database connection wrapper */
    class DbExecBase
    {
    public:
        /** create new db-connection. */
        DbExecBase(const string & i_connectionStr);

        /** close db-connection and cleanup connection resources. */
        ~DbExecBase(void) throw();

    protected:

        /** if true then transaction is active */
        bool isTrxActive;

        /** connection properties as key+value string pairs (case neutral). */
        NoCaseMap connProps;

        /** return string value of connection property by key (case neutral) or empty "" string if key not found. */
        string strConnProperty(const string & i_key);

        /** return true if value of connection property is 'true', 'yes', '1' (case neutral). */
        bool boolConnProperty(const string & i_key);

        /** return long value of connection property or default value if key not found (case neutral). */
        long long longConnProperty(const string & i_key, long i_default);

    private:
        /** parse connection string and return case-neutral map of key+value string pairs. */
        NoCaseMap parseConnectionStr(const string & i_connectionStr);

        // cleanup connection resources
        virtual void cleanup(void) throw() = 0;

        // validate connection properties
        virtual void validateConnectionProps(void) = 0;

    private:
        DbExecBase(const DbExecBase & i_dbExec);                // = delete;
        DbExecBase & operator=(const DbExecBase & i_dbExec);    // = delete;
    };
}

#endif  // DB_EXEC_BASE_H
