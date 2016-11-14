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

// db-provider names
static const char * dbProviderNameArr[] = {
    SQLITE_DB_PROVIDER, MYSQL_DB_PROVIDER, PGSQL_DB_PROVIDER, MSSQL_DB_PROVIDER, ORACLE_DB_PROVIDER, DB2_DB_PROVIDER
};
static const size_t dbProviderCount = sizeof(dbProviderNameArr) / sizeof(const char *);

/** max size of db table name */
static const int maxDbTableNameLen = 30;

/** db-exception default error message: "unknown db error" */
const char openm::dbUnknownErrorMessage[] = "unknown db error";

/** close db-connection and release connection resources. */
IDbExec::~IDbExec(void) throw() { }

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

/** return max length of db table or view name. */
int IDbExec::maxDbTableNameSize(const string & i_sqlProvider)
{
    if (i_sqlProvider == PGSQL_DB_PROVIDER) return 63;
    if (i_sqlProvider == ORACLE_DB_PROVIDER) return 30;
    return 64;
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

/** parse sql script text and return it as list of sql statements.
*
* @param[in] i_sqlScript    sql statements separated by ; semicolons
*
* @return   list of sql statements
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
list<string> IDbExec::parseSql(const string & i_sqlScript)
{
    try {
        // parse source string into list of sql statements
        list<string> stmtLst;

        string::size_type nPos = 0;
        string::size_type nLength = i_sqlScript.length();
        string stmt;
        stmt.reserve(OM_STR_DB_MAX);

        bool isQuote = false;   // inside of 'quote'
        bool isComment = false; // --comment until end of line
        char cPrev = '\0';
        char cNow = '\0';

        // until end of script parse each line 
        while (nPos < nLength) {

            // analyze current and previous char
            cPrev = cNow;
            cNow = i_sqlScript[nPos];

            if (!isComment && !isQuote && cNow == '-' && cPrev == '-') {

                if (!stmt.empty()) stmt.pop_back();     // remove - from the statement

                isComment = true;   // start of --comment: skip until end of line
                nPos++;             // move to next character
                continue;
            }

            if (!isComment && cNow == '\'') {
                isQuote = !isQuote;         // start or end of 'sql''quote'
                stmt += cNow;               // append to statement
                nPos++;                     // move to next character
                continue;
            }

            // if end-of-line then skip to the next line
            if (cNow == '\r' || cNow == '\n') {

                if (!stmt.empty()) stmt += '\x20';  // inside of sql replace all cr-lf with space

                // skip until next line started
                while (++nPos < nLength) {
                    cNow = i_sqlScript[nPos];
                    if (cNow != '\r' && cNow != '\n' &&
                        !isspace<char>(cNow, locale::classic()) && !iscntrl<char>(cNow, locale::classic())
                        ) break;
                }
                if (nPos >= nLength) break;   // done: end of script

                isComment = false;  // reset end of line state
                cPrev = '\0';
                continue;           // continue to next character
            }

            // if end of sql ; then append statment to the list
            if (!isComment && !isQuote && cNow == ';') {

                string s = trim(stmt);
                if (!s.empty()) {
                    stmtLst.push_back(s);
                }
                stmt.clear();

                isComment = false;  // reset end of sql state
                cPrev = '\0';
                cNow = '\0';
                nPos++;             // move to next character
                continue;
            }

            // if not --comment then append to statement
            if (!isComment) {
                // if non-printable except tab then replace with space
                if (cNow != '\t' &&
                    (isspace<char>(cNow, locale::classic()) || iscntrl<char>(cNow, locale::classic()))) {
                    cNow = '\x20';
                }
                stmt += cNow;
            }

            nPos++;     // move to next character
        }

        // append last statement if not empty
        stmt = trim(stmt);
        if (!stmt.empty()) stmtLst.push_back(stmt);

        return stmtLst;
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


