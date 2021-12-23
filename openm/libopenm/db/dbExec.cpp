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

/** MySQL and MariaDB db-provider name */
const char * openm::MYSQL_DB_PROVIDER = "mysql";

/** PostgreSQL db-provider name */
const char * openm::PGSQL_DB_PROVIDER = "postgresql";

/** MS SQL db-provider name */
const char * openm::MSSQL_DB_PROVIDER = "mssql";

/** Oracle db-provider name */
const char * openm::ORACLE_DB_PROVIDER = "oracle";

/** DB2 db-provider name */
const char * openm::DB2_DB_PROVIDER = "db2";

/** max length of db table or view name.
* Current max name sizes: PostgreSQL=63 MySQL=64 MSSQL=128 DB2=128 Oracle=128 (Oracle antiques not supported)
*/
const int IDbExec::maxDbTableNameSize = 63;

// db-provider names
static const char * dbProviderNameArr[] = {
    SQLITE_DB_PROVIDER, MYSQL_DB_PROVIDER, PGSQL_DB_PROVIDER, MSSQL_DB_PROVIDER, ORACLE_DB_PROVIDER, DB2_DB_PROVIDER
};
static const size_t dbProviderCount = sizeof(dbProviderNameArr) / sizeof(const char *);

/** db-exception default error message: "unknown db error" */
const char openm::dbUnknownErrorMessage[] = "unknown db error";

/** close db-connection and release connection resources. */
IDbExec::~IDbExec(void) noexcept { }

/** return list of provider names from supplied comma or semicolon separated string or exception on invalid name. */
list<string> IDbExec::parseListOfProviderNames(const string & i_sqlProviderNames)
{
    try {
        // parse list of provider names
        list<string> nameLst = splitCsv(i_sqlProviderNames, ",;");
        list<string> retLst;

        // convert provider name into lowercase
        // validate each name: it must be supported provider name
        // remove duplicates
        for (string & sName : nameLst) {
         
            if (sName.empty()) continue;
            toLower(sName);
            if (!isValidProviderName(sName.c_str())) throw DbException("invalid db-provider name: %s", sName.c_str());

            bool isFound = false;
            for (const string & sn : retLst) {
                isFound = sn == sName;
                if (isFound) break;
            }
            if (!isFound) retLst.push_back(sName);
        }

        return retLst;
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

/** return type name for BIGINT sql type */
string IDbExec::bigIntTypeName(const string & i_sqlProvider)
{
    if (i_sqlProvider == ORACLE_DB_PROVIDER) return "NUMBER(19)";
    return "BIGINT";
}

/** return type name for FLOAT standard sql type */
string IDbExec::floatTypeName(const string & i_sqlProvider)
{
    if (i_sqlProvider == ORACLE_DB_PROVIDER) return "BINARY_DOUBLE";
    return "FLOAT";
}

/** return column type DDL for long VARCHAR columns, use it for len > 255. */
string IDbExec::textTypeName(const string & i_sqlProvider, int i_size)
{
    try {
        if (i_sqlProvider == MSSQL_DB_PROVIDER && i_size > 4000) {
            return "TEXT";
        }
        if (i_sqlProvider == ORACLE_DB_PROVIDER && i_size > 2000) {
            return "CLOB";
        }
        if (i_sqlProvider == SQLITE_DB_PROVIDER ||
            i_sqlProvider == MYSQL_DB_PROVIDER ||
            i_sqlProvider == PGSQL_DB_PROVIDER ||
            i_sqlProvider == MSSQL_DB_PROVIDER ||
            i_sqlProvider == ORACLE_DB_PROVIDER ||
            i_sqlProvider == DB2_DB_PROVIDER) {
            return "VARCHAR(" + to_string(i_size) + ")";
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
string IDbExec::makeSqlCommitTransaction(const string & /*i_sqlProvider*/)
{
    return "COMMIT";
}

/**  make sql statement to create table if not exists. */
string IDbExec::makeSqlCreateTableIfNotExist(
    const string & i_sqlProvider, const string & i_tableName, const string & i_tableBodySql
)
{
    try {
        if (i_sqlProvider == SQLITE_DB_PROVIDER ||
            i_sqlProvider == MYSQL_DB_PROVIDER ||
            i_sqlProvider == PGSQL_DB_PROVIDER) {
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
        if (i_sqlProvider == MYSQL_DB_PROVIDER ||
            i_sqlProvider == PGSQL_DB_PROVIDER) {
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

// sorted list of SQL reserved keywords, it is incomplete by nature of SQL and mostly based on SQLite documentation
static const char * sqlKeywords[] = {
    "ABORT",
    "ABS",
    "ACTION",
    "ADD",
    "AFTER",
    "ALL",
    "ALTER",
    "ALWAYS",
    "ANALYZE",
    "AND",
    "ANY",
    "AS",
    "ASC",
    "ATTACH",
    "AUTOINCREMENT",
    "AVG",
    "BEFORE",
    "BEGIN",
    "BETWEEN",
    "BY",
    "CASCADE",
    "CASE",
    "CAST",
    "CHAR",
    "CHARACTER",
    "CHECK",
    "COLLATE",
    "COLUMN",
    "COMMIT",
    "CONFLICT",
    "CONSTRAINT",
    "CREATE",
    "CROSS",
    "CURRENT",
    "CURRENT_DATE",
    "CURRENT_TIME",
    "CURRENT_TIMESTAMP",
    "DATABASE",
    "DECIMAL",
    "DEFAULT",
    "DEFERRABLE",
    "DEFERRED",
    "DELETE",
    "DESC",
    "DETACH",
    "DISTINCT",
    "DO",
    "DROP",
    "EACH",
    "ELSE",
    "END",
    "ESCAPE",
    "EXCEPT",
    "EXCLUDE",
    "EXCLUSIVE",
    "EXISTS",
    "EXPLAIN",
    "FAIL",
    "FILTER",
    "FIRST",
    "FLOAT",
    "FOLLOWING",
    "FOR",
    "FOREIGN",
    "FROM",
    "FULL",
    "FUNCTION",
    "GENERATED",
    "GLOB",
    "GO",
    "GROUP",
    "GROUPS",
    "HAVING",
    "IF",
    "IGNORE",
    "IMMEDIATE",
    "IN",
    "INDEX",
    "INDEXED",
    "INITIALLY",
    "INNER",
    "INSERT",
    "INSTEAD",
    "INTERSECT",
    "INT",
    "INTEGER",
    "INTO",
    "IS",
    "ISNULL",
    "JOIN",
    "KEY",
    "LAST",
    "LEFT",
    "LIKE",
    "LIMIT",
    "MATCH",
    "MATERIALIZED",
    "MAX",
    "MIN",
    "MONEY",
    "NATURAL",
    "NO",
    "NOT",
    "NOTHING",
    "NOTNULL",
    "NULL",
    "NULLS",
    "NUMERIC",
    "OF",
    "OFFSET",
    "ON",
    "OR",
    "ORDER",
    "OTHERS",
    "OUTER",
    "OVER",
    "PARTITION",
    "PLAN",
    "PRAGMA",
    "PRECEDING",
    "PRECISION",
    "PRIMARY",
    "PROCEDURE",
    "PUBLIC",
    "QUERY",
    "RAISE",
    "RANGE",
    "REAL",
    "RECURSIVE",
    "REFERENCES",
    "REGEXP",
    "REINDEX",
    "RELEASE",
    "RENAME",
    "REPLACE",
    "RESTRICT",
    "RETURNING",
    "RIGHT",
    "ROLLBACK",
    "ROW",
    "ROWS",
    "SAVEPOINT",
    "SELECT",
    "SET",
    "SQRT",
    "SUM",
    "TABLE",
    "TEMP",
    "TEMPORARY",
    "THEN",
    "TIES",
    "TO",
    "TRANSACTION",
    "TRIGGER",
    "UNBOUNDED",
    "UNION",
    "UNIQUE",
    "UPDATE",
    "USING",
    "VACUUM",
    "VALUES",
    "VARCHAR",
    "VIEW",
    "VIRTUAL",
    "WHEN",
    "WHERE",
    "WINDOW",
    "WITH",
    "WITHOUT"
};
static const size_t sqlKeywordsCount = sizeof(sqlKeywords) / sizeof(const char *);

bool IDbExec::isSqlKeyword(const char * i_keyword, size_t i_length)
{
    return binary_search(
        sqlKeywords,
        sqlKeywords + sqlKeywordsCount,
        i_keyword, 
        [i_length](const char * i_left, const char * i_right) -> bool { return compareNoCase(i_left, i_right, i_length) < 0; }
    );
}
