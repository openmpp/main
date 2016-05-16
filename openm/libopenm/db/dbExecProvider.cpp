/**
 * @file
 * OpenM++ data library: base class for db connection wrapper
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "dbExec.h"
using namespace openm;

//
// Connection factory for each db-provider
//
#ifdef OM_DB_SQLITE

#include "dbExecSqlite.h"

/** db-connection factory: create new db-connection. */
IDbExec * IDbExec::create(const string & i_sqlProvider, const string & i_connectionStr)
{
    try {
        lock_guard<recursive_mutex> lck(dbMutex);

        if (i_sqlProvider == SQLITE_DB_PROVIDER) return new DbExecSqlite(i_connectionStr);
        throw 
            DbException("invalid db-provider name: %s", i_sqlProvider.c_str());
    }
    catch (DbException & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw;
    }
    catch (exception & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw DbException(ex.what());
    }
}

#else   // OM_DB_SQLITE
#error No database providers defined
#endif  // OM_DB_SQLITE
