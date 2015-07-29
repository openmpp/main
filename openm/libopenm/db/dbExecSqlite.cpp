/**
 * @file
 * OpenM++ data library: db connection wrapper for SQLite
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "dbExecSqlite.h"
using namespace openm;

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif  // _WIN32

/**
* create new db-connection.
*
* @param[in]   i_connectionStr database connection string.
*                          
* connection string example: \n
*   Database=fileName.sqlite; Timeout=86400; OpenMode=ReadWrite; DeleteExisting=true; \n
*   
* connection string options: \n
*  Database        (required) database file name or URI, it can be empty \n
*  Timeout         (optional) timeout in seconds for table lock, default=0 \n
*  OpenMode        (optional) open mode: ReadOnly, ReadWrite, Create, default=ReadOnly \n
*  DeleteExisting  (optional) if true the delete existing database file, default=false \n
*/
DbExecSqlite::DbExecSqlite(const string & i_connectionStr) :
    DbExecBase(i_connectionStr),
    theDb(NULL),
    theStmt(NULL)
{
    // release connection on failure
    exit_guard<DbExecSqlite> onExit(this, &DbExecSqlite::cleanup);

    try {
        lock_guard<recursive_mutex> lck(rtMutex);

        validateConnectionProps();  // exception if connection properties is invalid

        // if DeleteExisting=true then delete existing file
        string dbName = strConnProperty("Database");

        if (boolConnProperty("DeleteExisting") && !access(dbName.c_str(), 0)) {
            if (unlink(dbName.c_str())) 
                throw DbException("File delete error: %s", dbName.c_str());
        }

        // get open mode
        string sMode = strConnProperty("OpenMode");

        int openMode = SQLITE_OPEN_READONLY;
        if (!compareNoCase(sMode.c_str(), "ReadWrite")) openMode = SQLITE_OPEN_READWRITE;
        if (!compareNoCase(sMode.c_str(), "Create")) openMode = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;

        // open database file
        if (sqlite3_open_v2(dbName.c_str(), &theDb, openMode, NULL) != SQLITE_OK) throw DbException(sqlite3_errmsg(theDb));

        // register extension functions
        if (sqlite3_extension_functions_init(theDb) != SQLITE_OK) throw DbException(sqlite3_errmsg(theDb));

        // set table lock timeout, if specified
        if (sqlite3_busy_timeout(theDb, (int)(1000L * longConnProperty("Timeout", 0L)) ) != SQLITE_OK) throw DbException(sqlite3_errmsg(theDb));
    }
    catch (DbException & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw;
    }
    catch (exception & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw DbException(ex.what());
    }

    onExit.hold(); // completed OK
}

/** close db-connection and cleanup connection resources. */
DbExecSqlite::~DbExecSqlite(void) throw()
{
    try {
        lock_guard<recursive_mutex> lck(rtMutex);
        cleanup();
    }
    catch (...) { }
}

/** close db-connection and cleanup connection resources. */
void DbExecSqlite::cleanup(void) throw()
{
    try {
        lock_guard<recursive_mutex> lck(rtMutex);

        releaseStatement();
        try { releaseTransaction(); }
        catch (...) { }

        if (theDb != NULL) sqlite3_close(theDb);
        theDb = NULL;
    }
    catch (...) { }
}

/** release statement resources.
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
void DbExecSqlite::releaseStatement(void) throw()
{
    try {
        lock_guard<recursive_mutex> lck(rtMutex);

        if (theStmt != NULL) sqlite3_finalize(theStmt);
        theStmt = NULL;
    }
    catch (...) { }
}

// list of valid connection string keys
static const char * connPropKeyArr[] = { 
    "Database", "Timeout", "OpenMode", "DeleteExisting" 
};
static const size_t connPropKeySize = sizeof(connPropKeyArr) / sizeof(const char *);

// list of valid OpenMode values
static const char * openModePropArr[] = { 
    "ReadOnly", "ReadWrite", "Create" 
};

static const size_t openModePropSize = sizeof(openModePropArr) / sizeof(const char *);

/**
* validate connection properties.
*                          
* example: \n
*   Database=fileName.sqlite; Timeout=86400; OpenMode=ReadWrite; DeleteExisting=true; \n
*   
* syntax: \n
*  Database        (required) database file name or URI, it can be empty \n
*  Timeout         (optional) timeout in seconds for table lock, default=0 \n
*  OpenMode        (optional) open mode: ReadOnly, ReadWrite, Create, default=ReadOnly \n
*  DeleteExisting  (optional) if true the delete existing database file, default=false \n
*
*/
void DbExecSqlite::validateConnectionProps(void)
{
    // check: all keys must be in valid key list
    for (NoCaseMap::const_iterator propIt = connProps.cbegin(); propIt != connProps.cend(); ++propIt) {
        if (std::none_of(
            connPropKeyArr, 
            connPropKeyArr + connPropKeySize, 
            [propIt](const char * i_propKey) -> bool { return equalNoCase(i_propKey, propIt->first.c_str()); }
            ))
            throw DbException("Invalid key in connection string: %s", propIt->first.c_str());
    }

    // required: Database=; file name can be empty, but Database key must be part of connection string
    if (connProps.find("Database") == connProps.end()) throw DbException("Invalid connection string: missing Database=fileName.sqlite");

    // validate OpenMode value, if specified in connection string
    if (connProps.find("OpenMode") != connProps.end()) {

        string sMode = connProps["OpenMode"];

        if (sMode != "" && 
            std::none_of(
            openModePropArr, 
            openModePropArr + openModePropSize, 
            [sMode](const char * i_openMode) -> bool { return equalNoCase(i_openMode, sMode.c_str()); }
            ))
            throw DbException("Invalid connection string: invalid value of OpenMode=%s", sMode.c_str());
    }
}

/**
* select integer value of first (row,column) or default if no rows or value IS NULL.
*
* @param[in]   i_sql       select sql query
* @param[in]   i_default   default value if no row(s) selected or db-value IS NULL
*
* @return  integer value of first (row,column) or default if no rows or value IS NULL
*/
int DbExecSqlite::selectToInt(const string & i_sql, int i_default)
{
    return selectTo<int>(i_sql, i_default, &DbExecSqlite::fieldToInt<int>);
}

/**
* select long value of first (row,column) or default if no rows or value IS NULL.
*
* @param[in]   i_sql       select sql query
* @param[in]   i_default   default value if no row(s) selected or db-value IS NULL
*
* @return  long value of first (row,column) or default if no rows or value IS NULL
*/
long long DbExecSqlite::selectToLong(const string & i_sql, long long i_default)
{
    return selectTo<long long>(i_sql, i_default, &DbExecSqlite::fieldToInt<long long>);
}

/**
* select string value of first (row,column) or empty "" string if no rows or value IS NULL.
*
* @param[in]   i_sql       select sql query
*
* @return  string value of first (row,column) or empty "" string if no rows or value IS NULL
*/
string DbExecSqlite::selectToStr(const string & i_sql)
{
    return selectTo<string>(i_sql, "", &DbExecSqlite::fieldToStr);
}

/**
* select boolean value of first (row,column) or false if no rows or value IS NULL.
*
* @param[in]   i_sql       select sql query
*
* @return  boolean value of first (row,column) or false if no rows or value IS NULL
*/
bool DbExecSqlite::selectToBool(const string & i_sql)
{
    return selectTo<bool>(i_sql, 0, &DbExecSqlite::fieldToBool);
}

/**
* select double value of first (row,column) or default if no rows or value IS NULL.
*
* @param[in]   i_sql       select sql query
* @param[in]   i_default   default value if no row(s) selected or db-value IS NULL
*
* @return  double value of first (row,column) or default if no rows or value IS NULL
*/
double DbExecSqlite::selectToDouble(const string & i_sql, double i_default)
{
    return selectTo<double>(i_sql, i_default, &DbExecSqlite::fieldToDouble);
}

// Execute select query and return value of first (row,column) or default if no records or value is NULL
template <typename TCvt>
TCvt DbExecSqlite::selectTo(const string & i_sql, const TCvt & i_default, TCvt (DbExecSqlite::*ToRetType)(int))
{
    try {
        lock_guard<recursive_mutex> lck(rtMutex);

        if (theDb == NULL) throw DbException("db connection is closed");
        if (theStmt != NULL) throw DbException("db statement busy");
        if (isTransactionNonOwn()) throw DbException("db transaction active on other thread");
        theLog->logSql(i_sql.c_str());

        // release statement on exit
        exit_guard<DbExecSqlite> onExit(this, &DbExecSqlite::releaseStatement);

        // prepare and execute sql query
        if (sqlite3_prepare_v2(theDb, i_sql.c_str(), -1, &theStmt, NULL) != SQLITE_OK) throw DbException(sqlite3_errmsg(theDb));

        if (sqlite3_column_count(theStmt) <= 0) return i_default;   // if no columns in sql query

        int rc = sqlite3_step(theStmt);
        if (rc != SQLITE_OK && rc != SQLITE_DONE && rc != SQLITE_ROW) throw DbException(sqlite3_errmsg(theDb));

        // if no columns in current row or value is NULL
        if (sqlite3_data_count(theStmt) <= 0 || sqlite3_column_type(theStmt, 0) == SQLITE_NULL) return i_default;

        // retrieve first (row,column) value and check for errors
        return (this->*ToRetType)(0);
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

/**
* select string vector of first row or empty vector if no rows, empty "" string used for NULLs.
*
* @param[in]   i_sql       select sql query
*
* @return  string vector of first row or empty vector if no rows, empty "" string used for NULLs
*/
vector<string> DbExecSqlite::selectRowStr(const string & i_sql)
{
    try {
        lock_guard<recursive_mutex> lck(rtMutex);

        if (theDb == NULL) throw DbException("db connection is closed");
        if (theStmt != NULL) throw DbException("db statement busy");
        if (isTransactionNonOwn()) throw DbException("db transaction active on other thread");
        theLog->logSql(i_sql.c_str());

        // release statement on exit
        exit_guard<DbExecSqlite> onExit(this, &DbExecSqlite::releaseStatement);

        // prepare and execute sql query
        if (sqlite3_prepare_v2(theDb, i_sql.c_str(), -1, &theStmt, NULL) != SQLITE_OK) throw DbException(sqlite3_errmsg(theDb));

        int rowSize = sqlite3_column_count(theStmt);

        if (rowSize <= 0) return vector<string>();  // empty row: no columns in sql query

        int rc = sqlite3_step(theStmt);
        if (rc != SQLITE_OK && rc != SQLITE_DONE && rc != SQLITE_ROW) throw DbException(sqlite3_errmsg(theDb));

        if (sqlite3_data_count(theStmt) != rowSize) return vector<string>();    // empty row: no current row

        // retrieve current row values as strings
        vector<string> row;
        for (int nCol = 0; nCol < rowSize; nCol++) {
            row.push_back( (sqlite3_column_type(theStmt, nCol) != SQLITE_NULL) ? std::move(fieldToStr(nCol)) : "" );
        }

        return row;
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

/**
* select vector of rows, each row created and field values set by row adapter.
*
* @param[in]   i_sql       select sql query
* @param[in]   i_adapter   row adapter class to create rows and set values
*
* @return  vector of rows
*/
IRowBaseVec DbExecSqlite::selectRowList(const string & i_sql, const IRowAdapter & i_adapter)
{
    try {
        lock_guard<recursive_mutex> lck(rtMutex);

        if (theDb == NULL) throw DbException("db connection is closed");
        if (theStmt != NULL) throw DbException("db statement busy");
        if (isTransactionNonOwn()) throw DbException("db transaction active on other thread");
        theLog->logSql(i_sql.c_str());

        // release statement on exit
        exit_guard<DbExecSqlite> onExit(this, &DbExecSqlite::releaseStatement);

        // prepare and execute sql query
        if (sqlite3_prepare_v2(theDb, i_sql.c_str(), -1, &theStmt, NULL) != SQLITE_OK) throw DbException(sqlite3_errmsg(theDb));

        int rowSize = sqlite3_column_count(theStmt);

        if (rowSize <= 0) return IRowBaseVec();   // empty row: no columns in sql query

        // check db-row size: it must be the same as target row size
        if (rowSize != i_adapter.size()) 
            throw DbException("invalid db row size: sql query return %d columns, expected: %d", rowSize, i_adapter.size());

        // for each column find method to set field value
        const type_info ** colType = i_adapter.columnTypes();
        vector<SetFieldHandler> setterVec;

        for (int nCol = 0; nCol < rowSize; nCol++) {
            if (*(colType[nCol]) == typeid(char)) {
                setterVec.push_back(&DbExecSqlite::setFieldInt<char>);
            }
            else if (*(colType[nCol]) == typeid(unsigned char)) {
                setterVec.push_back(&DbExecSqlite::setFieldInt<unsigned char>);
            }
            else if (*(colType[nCol]) == typeid(short)) {
                setterVec.push_back(&DbExecSqlite::setFieldInt<short>);
            }
            else if (*(colType[nCol]) == typeid(unsigned short)) {
                setterVec.push_back(&DbExecSqlite::setFieldInt<unsigned short>);
            }
            else if (*(colType[nCol]) == typeid(int)) {
                setterVec.push_back(&DbExecSqlite::setFieldInt<int>);
            }
            else if (*(colType[nCol]) == typeid(unsigned int)) {
                setterVec.push_back(&DbExecSqlite::setFieldInt<unsigned int>);
            }
            else if (*(colType[nCol]) == typeid(long)) {
                setterVec.push_back(&DbExecSqlite::setFieldInt<long>);
            }
            else if (*(colType[nCol]) == typeid(unsigned long)) {
                setterVec.push_back(&DbExecSqlite::setFieldInt<unsigned long>);
            }
            else if (*(colType[nCol]) == typeid(long long)) {
                setterVec.push_back(&DbExecSqlite::setFieldInt<long long>);
            }
            else if (*(colType[nCol]) == typeid(unsigned long long)) {
                setterVec.push_back(&DbExecSqlite::setFieldInt<unsigned long long>);
            }
            else if (*(colType[nCol]) == typeid(int8_t)) {
                setterVec.push_back(&DbExecSqlite::setFieldInt<int8_t>);
            }
            else if (*(colType[nCol]) == typeid(uint8_t)) {
                setterVec.push_back(&DbExecSqlite::setFieldInt<uint8_t>);
            }
            else if (*(colType[nCol]) == typeid(int16_t)) {
                setterVec.push_back(&DbExecSqlite::setFieldInt<int16_t>);
            }
            else if (*(colType[nCol]) == typeid(uint16_t)) {
                setterVec.push_back(&DbExecSqlite::setFieldInt<uint16_t>);
            }
            else if (*(colType[nCol]) == typeid(int32_t)) {
                setterVec.push_back(&DbExecSqlite::setFieldInt<int32_t>);
            }
            else if (*(colType[nCol]) == typeid(uint32_t)) {
                setterVec.push_back(&DbExecSqlite::setFieldInt<uint32_t>);
            }
            else if (*(colType[nCol]) == typeid(int64_t)) {
                setterVec.push_back(&DbExecSqlite::setFieldInt<int64_t>);
            }
            else if (*(colType[nCol]) == typeid(uint64_t)) {
                setterVec.push_back(&DbExecSqlite::setFieldInt<uint64_t>);
            }
            else if (*(colType[nCol]) == typeid(bool)) {
                setterVec.push_back(&DbExecSqlite::setFieldBool);
            }
            else if (*(colType[nCol]) == typeid(float)) {
                setterVec.push_back(&DbExecSqlite::setFieldDbl<float>);
            }
            else if (*(colType[nCol]) == typeid(double)) {
                setterVec.push_back(&DbExecSqlite::setFieldDbl<double>);
            }
            else if (*(colType[nCol]) == typeid(long double)) {
                setterVec.push_back(&DbExecSqlite::setFieldDbl<long double>);
            }
            else if (*(colType[nCol]) == typeid(string)) {
                setterVec.push_back(&DbExecSqlite::setFieldStr);
            }
            else {
                throw DbException("invalid db column type");   // conversion to target field type is not supported
            }
        }

        // retrieve all rows
        IRowBaseVec rowVec;

        int rc = SQLITE_OK;
        do {
            // get next row, if available
            rc = sqlite3_step(theStmt);
            if (rc != SQLITE_OK && rc != SQLITE_DONE && rc != SQLITE_ROW) throw DbException(sqlite3_errmsg(theDb));

            if (rc != SQLITE_ROW) break;    // no more rows

            // retrieve current row values
            IRowBaseUptr row(i_adapter.createRow());

            for (int nCol = 0; nCol < rowSize; nCol++) {

                // if field value IS NULL then use default
                if (sqlite3_column_type(theStmt, nCol) == SQLITE_NULL) continue;

                // convert field db-value to target type and set field value
                (this->*(setterVec[nCol]))(row.get(), nCol, i_adapter);
            }

            rowVec.push_back(std::move(row));   // append to output row list
        }
        while (rc == SQLITE_ROW);
        
        return rowVec;
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
long long DbExecSqlite::selectColumn(const string & i_sql, int i_column, const type_info & i_type, long long i_size, void * io_valueArr)
{
    try {
        lock_guard<recursive_mutex> lck(rtMutex);

        if (theDb == NULL) throw DbException("db connection is closed");
        if (theStmt != NULL) throw DbException("db statement busy");
        if (isTransactionNonOwn()) throw DbException("db transaction active on other thread");
        theLog->logSql(i_sql.c_str());

        // validate parameters: at least one value expected
        if (i_size <= 0 || io_valueArr == NULL) throw DbException("invalid parameter(s): row count or buffer is nullptr");

        // release statement on exit
        exit_guard<DbExecSqlite> onExit(this, &DbExecSqlite::releaseStatement);

        // prepare and execute sql query
        if (sqlite3_prepare_v2(theDb, i_sql.c_str(), -1, &theStmt, NULL) != SQLITE_OK) throw DbException(sqlite3_errmsg(theDb));

        int rowSize = sqlite3_column_count(theStmt);

        if (rowSize <= 0) throw DbException("invalid db row size: sql query return %d columns, expected: 1", rowSize);  // empty row: no columns in sql query

        // retrieve specified number of rows from one column
        if (i_type == typeid(char)) return retrieveColumnTo<char>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToInt<char>);
        if (i_type == typeid(unsigned char)) return retrieveColumnTo<unsigned char>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToInt<unsigned char>);
        if (i_type == typeid(short)) return retrieveColumnTo<short>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToInt<short>);
        if (i_type == typeid(unsigned short)) return retrieveColumnTo<unsigned short>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToInt<unsigned short>);
        if (i_type == typeid(int)) return retrieveColumnTo<int>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToInt<int>);
        if (i_type == typeid(unsigned int)) return retrieveColumnTo<unsigned int>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToInt<unsigned int>);
        if (i_type == typeid(long)) return retrieveColumnTo<long>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToInt<long>);
        if (i_type == typeid(unsigned long)) return retrieveColumnTo<unsigned long>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToInt<unsigned long>);
        if (i_type == typeid(long long)) return retrieveColumnTo<long long>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToInt<long long>);
        if (i_type == typeid(unsigned long long)) return retrieveColumnTo<unsigned long long>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToInt<unsigned long long>);
        if (i_type == typeid(int8_t)) return retrieveColumnTo<int8_t>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToInt<int8_t>);
        if (i_type == typeid(uint8_t)) return retrieveColumnTo<uint8_t>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToInt<uint8_t>);
        if (i_type == typeid(int16_t)) return retrieveColumnTo<int16_t>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToInt<int16_t>);
        if (i_type == typeid(uint16_t)) return retrieveColumnTo<uint16_t>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToInt<uint16_t>);
        if (i_type == typeid(int32_t)) return retrieveColumnTo<int32_t>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToInt<int32_t>);
        if (i_type == typeid(uint32_t)) return retrieveColumnTo<uint32_t>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToInt<uint32_t>);
        if (i_type == typeid(int64_t)) return retrieveColumnTo<int64_t>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToInt64);
        if (i_type == typeid(uint64_t)) return retrieveColumnTo<uint64_t>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToInt<uint64_t>);
        if (i_type == typeid(bool)) return retrieveColumnTo<bool>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToBool);
        if (i_type == typeid(float)) return retrieveColumnTo<float>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToDbl<float>);
        if (i_type == typeid(double)) return retrieveColumnTo<double>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToDouble);
        if (i_type == typeid(long double)) return retrieveColumnTo<long double>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToDbl<long double>);
        if (i_type == typeid(string)) return retrieveColumnTo<string>(i_column, i_size, io_valueArr, &DbExecSqlite::fieldToStr);

        throw DbException("invalid db column type");   // conversion to target field type is not supported
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

// Retrieve single column field values into io_valueArray[i_size] buffer and return row count
template <typename TCol>
long long DbExecSqlite::retrieveColumnTo(int i_column, long long i_size, void * io_valueArr, TCol (DbExecSqlite::*ToRetType)(int))
{
long long nRow = 0;
int rc = SQLITE_ROW;

    for (nRow = 0; nRow < i_size && rc == SQLITE_ROW; nRow++) {

        // get next row, if available
        rc = sqlite3_step(theStmt);
        if (rc != SQLITE_OK && rc != SQLITE_DONE && rc != SQLITE_ROW) throw DbException(sqlite3_errmsg(theDb));

        if (rc != SQLITE_ROW) break;    // no more rows

        // if field value IS NULL then use default
        if (sqlite3_column_type(theStmt, i_column) == SQLITE_NULL) continue;

        // convert field db-value to target type and set field value
        static_cast<TCol *>(io_valueArr)[nRow] = std::move( (this->*ToRetType)(i_column) );
    }

    if (nRow != i_size) throw DbException("no data found: expected %lld rows, selected: %lld", i_size, nRow);   // row count less than expected
        
    return nRow;
}

/**
* execute sql statement (update, insert, delete, create, etc).
*
* @param[in] i_sql     sql statement of "update" type (update, insert, delete, create, etc.)
*
* @return   number of affected rows (db-vendor specific).
*/
long long DbExecSqlite::update(const string & i_sql)
{
    try {
        lock_guard<recursive_mutex> lck(rtMutex);

        if (theDb == NULL) throw DbException("db connection is closed");
        if (theStmt != NULL) throw DbException("db statement busy");
        if (isTransactionNonOwn()) throw DbException("db transaction active on other thread");
        theLog->logSql(i_sql.c_str());

        // execute sql and ignore any results
        if (sqlite3_exec(theDb, i_sql.c_str(), NULL, NULL, NULL) != SQLITE_OK) throw DbException(sqlite3_errmsg(theDb));

        return sqlite3_changes(theDb);  // affected rows count for last update(able) statement
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

/** begin transaction, throw exception if transaction already active or statement is active. */
void DbExecSqlite::beginTransaction(void)
{
    try {
        lock_guard<recursive_mutex> lck(rtMutex);

        if (theDb == NULL) throw DbException("db connection is closed");
        if (theStmt != NULL) throw DbException("db statement busy");
        if (isTransaction()) throw DbException("db transaction is already active");

        const char * sql = "BEGIN TRANSACTION";
        theLog->logSql(sql);

        // execute sql and ignore any results
        if (sqlite3_exec(theDb, sql, NULL, NULL, NULL) != SQLITE_OK) throw DbException(sqlite3_errmsg(theDb));

        setTransactionActive();     // transaction is active
        return;
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

/** begin transaction in multi-threaded environment, throw exception if transaction already active or statement is active. */
unique_lock<recursive_mutex> DbExecSqlite::beginTransactionThreaded(void)
{
    try {
        unique_lock<recursive_mutex> lck(rtMutex);
        beginTransaction();
        return lck;
    }
    catch (DbException & /* ex */) {
        throw;
    }
    catch (exception & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw DbException(ex.what());
    }
}

/** commit transaction, does nothing if no active transaction, throw exception if statement is active. */
void DbExecSqlite::commit(void)
{
    try {
        lock_guard<recursive_mutex> lck(rtMutex);

        if (theDb == NULL) throw DbException("db connection is closed");
        if (theStmt != NULL) throw DbException("db statement busy");
        if (isTransactionNonOwn()) throw DbException("db transaction active on other thread");

        if (!isTransaction()) return;   // no active transaction - nothing to do

        const char * sql = "COMMIT";
        theLog->logSql(sql);

        // execute sql and ignore any results
        if (sqlite3_exec(theDb, sql, NULL, NULL, NULL) != SQLITE_OK) throw DbException(sqlite3_errmsg(theDb));

        releaseTransaction();           // release active transaction
        return;
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

/** rollback transaction, does nothing if no active transaction, throw exception if statement is active. */
void DbExecSqlite::rollback(void)
{
    try {
        lock_guard<recursive_mutex> lck(rtMutex);

        if (theDb == NULL) throw DbException("db connection is closed");
        if (theStmt != NULL) throw DbException("db statement busy");
        if (isTransactionNonOwn()) throw DbException("db transaction active on other thread");

        if (!isTransaction()) return;   // no active transaction - nothing to do

        const char * sql = "ROLLBACK";
        theLog->logSql(sql);

        // execute sql and ignore any results
        if (sqlite3_exec(theDb, sql, NULL, NULL, NULL) != SQLITE_OK) throw DbException(sqlite3_errmsg(theDb));

        releaseTransaction();           // release active transaction
        return;
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

/**
* create new statement with specified parameters.
*
* @param[in] i_sql        sql to create statement
* @param[in] i_paramCount number of parameters
* @param[in] i_typeArr    array of parameters type
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
void DbExecSqlite::createStatement(const string & i_sql, int i_paramCount, const type_info ** i_typeArr)
{
    try {
        lock_guard<recursive_mutex> lck(rtMutex);

        if (theDb == NULL) throw DbException("db connection is closed");
        if (theStmt != NULL) throw DbException("db statement busy");
        if (isTransactionNonOwn()) throw DbException("db transaction active on other thread");
        theLog->logSql(i_sql.c_str());

        // release statement on exit
        exit_guard<DbExecSqlite> onExit(this, &DbExecSqlite::releaseStatement);

        // validate method parameters
        if (i_paramCount <= 0 || i_paramCount > SHRT_MAX || i_typeArr == NULL) throw DbException("invalid number of parameters or type array is nullptr");

        // prepare sql statement and check number of parameters
        if (sqlite3_prepare_v2(theDb, i_sql.c_str(), -1, &theStmt, NULL) != SQLITE_OK) throw DbException(sqlite3_errmsg(theDb));

        int nSize = sqlite3_bind_parameter_count(theStmt);
        if (nSize != i_paramCount) throw DbException("invalid number of sql parameters");

        // for each parameter find bind method by source value type
        bindFncVec.clear();
        for (int k = 0; k < i_paramCount; k++) {
        
            const type_info * ti = i_typeArr[k];
            if (ti == NULL) throw DbException("invalid type to use as sql parameter");

            BindHandler bindFnc = NULL;
            if (*ti == typeid(char)) bindFnc = &DbExecSqlite::bindLong;
            if (*ti == typeid(unsigned char)) bindFnc = &DbExecSqlite::bindLong;
            if (*ti == typeid(short)) bindFnc = &DbExecSqlite::bindLong;
            if (*ti == typeid(unsigned short)) bindFnc = &DbExecSqlite::bindLong;
            if (*ti == typeid(int)) bindFnc = &DbExecSqlite::bindLong;
            if (*ti == typeid(unsigned int)) bindFnc = &DbExecSqlite::bindLong;
            if (*ti == typeid(long)) bindFnc = &DbExecSqlite::bindLong;
            if (*ti == typeid(unsigned long)) bindFnc = &DbExecSqlite::bindLong;
            if (*ti == typeid(long long)) bindFnc = &DbExecSqlite::bindLong;
            if (*ti == typeid(unsigned long long)) bindFnc = &DbExecSqlite::bindLong;
            if (*ti == typeid(int8_t)) bindFnc = &DbExecSqlite::bindLong;
            if (*ti == typeid(uint8_t)) bindFnc = &DbExecSqlite::bindLong;
            if (*ti == typeid(int16_t)) bindFnc = &DbExecSqlite::bindLong;
            if (*ti == typeid(uint16_t)) bindFnc = &DbExecSqlite::bindLong;
            if (*ti == typeid(int32_t)) bindFnc = &DbExecSqlite::bindLong;
            if (*ti == typeid(uint32_t)) bindFnc = &DbExecSqlite::bindLong;
            if (*ti == typeid(int64_t)) bindFnc = &DbExecSqlite::bindLong;
            if (*ti == typeid(uint64_t)) bindFnc = &DbExecSqlite::bindLong;
            if (*ti == typeid(bool)) bindFnc = &DbExecSqlite::bindBool;
            if (*ti == typeid(float)) bindFnc = &DbExecSqlite::bindDbl;
            if (*ti == typeid(double)) bindFnc = &DbExecSqlite::bindDbl;
            if (*ti == typeid(long double)) bindFnc = &DbExecSqlite::bindDbl;
            // if (ti == typeid(string)) not supported

            if (bindFnc == NULL) throw DbException("invalid type to use as sql parameter"); // conversion to target parameter type is not supported

            bindFncVec.push_back(bindFnc);
        }

        // statement creation completed OK
        onExit.hold();
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

/**
* execute statement with parameters.
*
* @param[in] i_paramCount number of parameters
* @param[in] i_valueArr   array of parameters value
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
void DbExecSqlite::executeStatement(int i_paramCount, const DbValue * i_valueArr)
{
    try {
        lock_guard<recursive_mutex> lck(rtMutex);

        if (theDb == NULL) throw DbException("db connection is closed");
        if (theStmt == NULL) throw DbException("db statement not created");
        if (isTransactionNonOwn()) throw DbException("db transaction active on other thread");

        // validate parameters
        if (bindFncVec.size() != (size_t)i_paramCount || i_valueArr == NULL) throw DbException("invalid number of parameter values or data is nullptr");

        // set parameter values for the statement
        for (int nParam = 0; nParam < i_paramCount; nParam++) {
            (this->*(bindFncVec[nParam]))(nParam + 1, i_valueArr[nParam]);
        }

        // execute statement
        int rc = sqlite3_step(theStmt);
        if (rc != SQLITE_OK && rc != SQLITE_DONE && rc != SQLITE_ROW) throw DbException(sqlite3_errmsg(theDb));
        if (sqlite3_reset(theStmt) != SQLITE_OK) throw DbException(sqlite3_errmsg(theDb));
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

// Convert field of current row to string, return empty "" string if NULL
string DbExecSqlite::fieldToStr(int i_column)
{
    const char * s = reinterpret_cast<const char *>(sqlite3_column_text(theStmt, i_column));

    int rc = sqlite3_errcode(theDb);
    if (rc != SQLITE_OK && rc != SQLITE_DONE && rc != SQLITE_ROW) throw DbException(sqlite3_errmsg(theDb));

    int nSize = sqlite3_column_bytes(theStmt, i_column);

    return (s != NULL && nSize > 0) ? string(s, nSize) : "";
}

// Convert field of current row to integer
template<typename TInt> TInt DbExecSqlite::fieldToInt(int i_column)
{
    return (TInt)fieldToInt64(i_column);
}

// Convert field of current row to integer
int64_t DbExecSqlite::fieldToInt64(int i_column)
{
    int64_t nRet = sqlite3_column_int64(theStmt, i_column);

    int rc = sqlite3_errcode(theDb);
    if (rc != SQLITE_OK && rc != SQLITE_DONE && rc != SQLITE_ROW) throw DbException(sqlite3_errmsg(theDb));

    return nRet;
}

// Convert field of current row to double
template<typename TDbl> TDbl DbExecSqlite::fieldToDbl(int i_column)
{
    return (TDbl)fieldToDouble(i_column);
}

// Convert field of current row to double
double DbExecSqlite::fieldToDouble(int i_column)
{
    double dRet = sqlite3_column_double(theStmt, i_column);

    int rc = sqlite3_errcode(theDb);
    if (rc != SQLITE_OK && rc != SQLITE_DONE && rc != SQLITE_ROW) throw DbException(sqlite3_errmsg(theDb));

    return dRet;
}

// Convert field of current row to boolean
bool DbExecSqlite::fieldToBool(int i_column)
{
    return fieldToInt64(i_column) != 0;
}

// set integer field value by using row adapter
template<typename TInt>  
void DbExecSqlite::setFieldInt(IRowBase * i_row, int i_column, const IRowAdapter & i_adapter)
{
    TInt iVal = fieldToInt<TInt>(i_column);
    i_adapter.set(i_row, i_column, &iVal);
}

// set double field value by using row adapter
template<typename TDbl>  
void DbExecSqlite::setFieldDbl(IRowBase * i_row, int i_column, const IRowAdapter & i_adapter)
{
    TDbl dVal = fieldToDbl<TDbl>(i_column);
    i_adapter.set(i_row, i_column, &dVal);
}

// set boolean field value by using row adapter
void DbExecSqlite::setFieldBool(IRowBase * i_row, int i_column, const IRowAdapter & i_adapter)
{
    bool isVal = fieldToBool(i_column);
    i_adapter.set(i_row, i_column, &isVal);
}

// set string field value by using row adapter
void DbExecSqlite::setFieldStr(IRowBase * i_row, int i_column, const IRowAdapter & i_adapter)
{
    i_adapter.set(i_row, i_column, fieldToStr(i_column).c_str());
}

// bind integer sql parameter at specified position
void DbExecSqlite::bindLong(int i_position, const DbValue & i_value)
{
    sqlite3_int64 nVal = static_cast<sqlite3_int64>(i_value.llVal);
    if (sqlite3_bind_int64(theStmt, i_position, nVal) != SQLITE_OK) throw DbException(sqlite3_errmsg(theDb));
}

// bind double sql parameter at specified position, use NULL if double value not is finite
void DbExecSqlite::bindDbl(int i_position, const DbValue & i_value)
{
    double dVal = static_cast<double>(i_value.dVal);

    if (isfinite(dVal)) {
        if (sqlite3_bind_double(theStmt, i_position, dVal) != SQLITE_OK) throw DbException(sqlite3_errmsg(theDb));
    }
    else {
        if (sqlite3_bind_null(theStmt, i_position) != SQLITE_OK) throw DbException(sqlite3_errmsg(theDb));
    }
}

// bind bool sql parameter at specified position, use integer one if value is true else use zero
void DbExecSqlite::bindBool(int i_position, const DbValue & i_value)
{
    sqlite3_int64 nVal = i_value.isVal ? 1 : 0;
    if (sqlite3_bind_int64(theStmt, i_position, nVal) != SQLITE_OK) throw DbException(sqlite3_errmsg(theDb));
}
