/**
 * @file
 * OpenM++ data library: db connection wrapper public interface
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "helper.h"
#include "dbExec.h"
using namespace openm;

/** SQLite db-provider name */
const char * openm::SQLITE_DB_PROVIDER = "sqlite";

/** PostgreSQL db-provider name */
const char * openm::PGSQL_DB_PROVIDER = "postgresql";

/** MySQL db-provider name */
const char * openm::MYSQL_DB_PROVIDER = "mysql";

/** MS SQL db-provider name */
const char * openm::MSSQL_DB_PROVIDER = "mssql";

/** Oracle db-provider name */
const char * openm::ORACLE_DB_PROVIDER = "oracle";

/** DB2 db-provider name */
const char * openm::DB2_DB_PROVIDER = "db2";

// db-provider names
static const char * dbProviderNameArr[] = {
    SQLITE_DB_PROVIDER, PGSQL_DB_PROVIDER, MYSQL_DB_PROVIDER, MSSQL_DB_PROVIDER, ORACLE_DB_PROVIDER, DB2_DB_PROVIDER
};
static const size_t dbProviderCount = sizeof(dbProviderNameArr) / sizeof(const char *);

/** max size of db table name */
static const int maxDbTableNameLen = 30;

/** db-exception default error message: "unknown db error" */
const char openm::dbUnknownErrorMessage[] = "unknown db error";

/** close db-connection and release connection resources. */
IDbExec::~IDbExec(void) throw() { }

/** return list of provider names from supplied comma or semicolon separated string or exception on invalid name or empty list. */
list<string> IDbExec::parseListOfProviderNames(const string & i_sqlProviderNames)
{
    try {
        // parse list of provider names
        list<string> nameLst = splitCsv(i_sqlProviderNames, ",;");

        // convert provider name into lowercase
        // validate each name in the list and check if list is not empty
        for (string & sName : nameLst) {
            toLower(sName);
            if (!isValidProviderName(sName.c_str())) throw DbException("invalid db-provider name: %s", sName.c_str());
        }
        if (nameLst.empty()) throw DbException("invalid (empty) list db-provider names");

        return nameLst;
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

/** check if provider name is valid. */
bool IDbExec::isValidProviderName(const char * i_sqlProvider)
{
    return std::any_of(
        dbProviderNameArr,
        dbProviderNameArr + dbProviderCount,
        [&i_sqlProvider](const char * i_name) -> bool { return equalNoCase(i_name, i_sqlProvider); }
    );
}

/** return max length of db table or view name. */
int IDbExec::maxDbTableNameSize(const string & i_sqlProvider)
{
    if (i_sqlProvider == PGSQL_DB_PROVIDER) return 63;
    if (i_sqlProvider == ORACLE_DB_PROVIDER) return 30;
    return 64;
}

/** return sql statement to begin transaction (db-provider specific). */
string IDbExec::makeSqlBeginTransaction(const string & i_sqlProvider)
{
    try {
        if (i_sqlProvider == SQLITE_DB_PROVIDER || 
            i_sqlProvider == PGSQL_DB_PROVIDER || 
            i_sqlProvider == MSSQL_DB_PROVIDER) { 
            return "BEGIN TRANSACTION";
        }
        if (i_sqlProvider == MYSQL_DB_PROVIDER) { 
            return "START TRANSACTION";
        }
        if (i_sqlProvider == ORACLE_DB_PROVIDER ||
            i_sqlProvider == DB2_DB_PROVIDER) { 
            return "-- BEGIN TRANSACTION";
        }
        throw DbException("invalid db-provider name: %s", i_sqlProvider.c_str());
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

/** return sql statement to commit transaction (db-provider specific). */
string IDbExec::makeSqlCommitTransaction(const string & i_sqlProvider)
{
    try {
        if (i_sqlProvider == SQLITE_DB_PROVIDER || 
            i_sqlProvider == PGSQL_DB_PROVIDER || 
            i_sqlProvider == MYSQL_DB_PROVIDER || 
            i_sqlProvider == MSSQL_DB_PROVIDER ||
            i_sqlProvider == ORACLE_DB_PROVIDER ||
            i_sqlProvider == DB2_DB_PROVIDER) { 
            return "COMMIT";
        }
        throw DbException("invalid db-provider name: %s", i_sqlProvider.c_str());
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

/**  make sql statement to create table if not exists. */
string IDbExec::makeSqlCreateTableIfNotExist(
    const string & i_sqlProvider, const string & i_tableName, const string & i_tableBodySql
)
{
    try {
        if (i_sqlProvider == SQLITE_DB_PROVIDER ||
            i_sqlProvider == PGSQL_DB_PROVIDER ||
            i_sqlProvider == MYSQL_DB_PROVIDER) {
            return "CREATE TABLE IF NOT EXISTS " + i_tableName + " " + i_tableBodySql;
        }
        if (i_sqlProvider == MSSQL_DB_PROVIDER) {
            return
                "IF NOT EXISTS" \
                " (SELECT * FROM INFORMATION_SCHEMA.TABLES T WHERE T.TABLE_NAME = " + toQuoted(i_tableName) + ")\n" +
                " CREATE TABLE " + i_tableName + " " + i_tableBodySql;
        }
        if (i_sqlProvider == ORACLE_DB_PROVIDER ||
            i_sqlProvider == DB2_DB_PROVIDER) { 
            return "CREATE TABLE " + i_tableName + " " + i_tableBodySql;
        }
        throw DbException("invalid db-provider name: %s", i_sqlProvider.c_str());
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

/** make sql statement to create or replace view. */
string IDbExec::makeSqlCreateViewReplace(
    const string & i_sqlProvider, const string & i_viewName, const string & i_viewBodySql
)
{
    try {
        if (i_sqlProvider == SQLITE_DB_PROVIDER) {
            return "CREATE VIEW IF NOT EXISTS " + i_viewName + " AS " + i_viewBodySql;
        }
        if (i_sqlProvider == PGSQL_DB_PROVIDER ||
            i_sqlProvider == MYSQL_DB_PROVIDER) {
            return "CREATE OR REPLACE VIEW " + i_viewName + " AS " + i_viewBodySql;
        }
        if (i_sqlProvider == MSSQL_DB_PROVIDER) {
            return
                "IF EXISTS" \
                " (SELECT * FROM INFORMATION_SCHEMA.VIEWS V WHERE V.TABLE_NAME = " + toQuoted(i_viewName) + ")\n" +
                " DROP VIEW " + i_viewName + ";\n" \
                " GO\n"
                " CREATE VIEW " + i_viewName + " AS " + i_viewBodySql + "\n" +
                " GO";
        }
        if (i_sqlProvider == ORACLE_DB_PROVIDER ||
            i_sqlProvider == DB2_DB_PROVIDER) { 
            return "CREATE OR REPLACE VIEW " + i_viewName + " AS " + i_viewBodySql;
        }
        throw DbException("invalid db-provider name: %s", i_sqlProvider.c_str());
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
