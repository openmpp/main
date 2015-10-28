// OpenM++ data library: run_option table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // run_option table implementation
    class RunOptionTable : public IRunOptionTable
    {
    public:
        RunOptionTable(IDbExec * i_dbExec, int i_runId = 0);
        RunOptionTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~RunOptionTable() throw();

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by primary key: run id, option key
        const RunOptionRow * byKey(int i_runId, const string & i_key) const;

        // get list of all table rows
        vector<RunOptionRow> rows(void) const { return IMetaTable<RunOptionRow>::rows(rowVec); }

        // get list of rows by run id
        vector<RunOptionRow> byRunId(int i_runId) const;

        /** return true if primary key (run id, option key) found. */
        bool isExist(int i_runId, const char * i_key) const throw();
        
        /** return true if option key found. */
        bool isExist(const char * i_key) const throw();

        /** return string value by primary key (run id, option key) or default value if not found. */
        string strValue(int i_runId, const char * i_key, const string & i_default = "") const throw() { return strValue(true, i_runId, i_key, i_default); }

        /** return string value by option key (first found) or default value if not found. */
        string strValue(const char * i_key, const string & i_default = "") const throw() { return strValue(false, 0, i_key, i_default); }

        /** return boolean value by primary key (run id, option key) or false if not found or value not "yes", "1", "true". */
        bool boolValue(int i_runId, const char * i_key) const throw() { return boolValue(true, i_runId, i_key); }

        /** return boolean value by option key (first found) or false if not found or value not "yes", "1", "true". */
        bool boolValue(const char * i_key) const throw() { return boolValue(false, 0, i_key); }

        /** return long value by primary key (run id, option key) or default if not found or can not be converted to long. */
        long long longValue(int i_runId, const char * i_key, long long i_default) const throw() { return longValue(true, i_runId, i_key, i_default); }

        /** return long value by option key (first found) or default if not found or can not be converted to long. */
        long long longValue(const char * i_key, long long i_default) const throw() { return longValue(false, 0, i_key, i_default); }

        /** return double value by primary key (run id, option key) or default if not found or can not be converted to double. */
        double doubleValue(int i_runId, const char * i_key, double i_default) const throw() { return doubleValue(true, i_runId, i_key, i_default); }

        /** return double value by option key (first found) or default if not found or can not be converted to double. */
        double doubleValue(const char * i_key, double i_default) const throw() { return doubleValue(false, 0, i_key, i_default); }

    private:
        IRowBaseVec rowVec;     // table rows

        // Find first row by option key
        const RunOptionRow * firstOptionKey(const string & i_key) const;

        // return string value by primary key (run id, option key) or by option key only, return default value if not found
        string strValue(bool i_useRunId, int i_runId, const char * i_key, const string & i_default = "") const throw();

        // return boolean value by primary key (run id, option key) or by option key only, return false if not found or value not "yes", "1", "true"
        bool boolValue(bool i_useRunId, int i_runId, const char * i_key) const throw();

        // return long value by primary key (run id, option key) or by option key only, return default if not found or can not be converted to long
        long long longValue(bool i_useRunId, int i_runId, const char * i_key, long long i_default) const throw();

        // return double value by primary key (run id, option key) or by option key only, return default if not found or can not be converted to double
        double doubleValue(bool i_useRunId, int i_runId, const char * i_key, double i_default) const throw();

    private:
        RunOptionTable(const RunOptionTable & i_table) = delete;
        RunOptionTable & operator=(const RunOptionTable & i_table) = delete;
    };

    // Columns type for run_option row
    static const type_info * typeRunOptionRow[] = { 
        &typeid(decltype(RunOptionRow::runId)), 
        &typeid(decltype(RunOptionRow::key)), 
        &typeid(decltype(RunOptionRow::value))
    };

    // Size (number of columns) for run_option row
    static const int sizeRunOptionRow = sizeof(typeRunOptionRow) / sizeof(const type_info *);

    // Row adapter to select run_option rows
    class RunOptionRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new RunOptionRow(); }
        int size(void) const { return sizeRunOptionRow; }
        const type_info ** columnTypes(void) const { return typeRunOptionRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<RunOptionRow *>(i_row)->runId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<RunOptionRow *>(i_row)->key = ((const char *)i_value);
                break;
            case 2:
                dynamic_cast<RunOptionRow *>(i_row)->value = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IRunOptionTable::~IRunOptionTable(void) throw() { }

// Create new table rows by loading db rows
IRunOptionTable * IRunOptionTable::create(IDbExec * i_dbExec, int i_runId)
{
    return new RunOptionTable(i_dbExec, i_runId);
}

// Create new table rows by swap with supplied vector of rows
IRunOptionTable * IRunOptionTable::create(IRowBaseVec & io_rowVec)
{
    return new RunOptionTable(io_rowVec);
}

// Load table
RunOptionTable::RunOptionTable(IDbExec * i_dbExec, int i_runId)
{ 
    const IRowAdapter & adp = RunOptionRowAdapter();
    rowVec = IMetaTable<RunOptionRow>::load(
        "SELECT run_id, option_key, option_value FROM run_option" +
        ((i_runId > 0) ? " WHERE run_id = " + to_string(i_runId) : "") +
        " ORDER BY 1, 2", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
RunOptionTable::~RunOptionTable(void) throw() { }

// Find row by primary key: run id, option key
const RunOptionRow * RunOptionTable::byKey(int i_runId, const string & i_key) const
{
    const IRowBaseUptr keyRow( new RunOptionRow(i_runId, i_key) );
    return IMetaTable<RunOptionRow>::byKey(keyRow, rowVec);
}

// Find first row by option key
const RunOptionRow * RunOptionTable::firstOptionKey(const string & i_key) const
{
    IRowBaseVec::const_iterator rowIt =
        find_if(
        rowVec.cbegin(),
        rowVec.cend(),
        [i_key](const IRowBaseUptr & i_row) -> bool { return dynamic_cast<RunOptionRow *>(i_row.get())->key == i_key; }
    );
    return
        (rowIt != rowVec.cend()) ? dynamic_cast<RunOptionRow *>(rowIt->get()) : NULL;
}

// get list of rows by run id
vector<RunOptionRow> RunOptionTable::byRunId(int i_runId) const
{
    const IRowBaseUptr row( new RunOptionRow(i_runId, "") );
    return IMetaTable<RunOptionRow>::findAll(row, rowVec, RunOptionRow::runIdEqual);
}

/** return true if primary key (run id, option key) found. */
bool RunOptionTable::isExist(int i_runId, const char * i_key) const throw()
{ 
    try {
        return i_key != NULL && byKey(i_runId, i_key) != NULL;
    }
    catch (...) { 
        return false;
    }
}

/** return true if option key found. */
bool RunOptionTable::isExist(const char * i_key) const throw()
{
    try {
        return i_key != NULL && firstOptionKey(i_key) != NULL;
    }
    catch (...) {
        return false;
    }
}

// return string value by primary key (run id, option key) or by option key only, return default value if not found
string RunOptionTable::strValue(bool i_useRunId, int i_runId, const char * i_key, const string & i_default) const throw()
{
    try {
        if (i_key == NULL) return i_default;    // invalid key

        const RunOptionRow * optRow = i_useRunId ? byKey(i_runId, i_key) : firstOptionKey(i_key);
        return
            (optRow != NULL) ? optRow->value : i_default;
    }
    catch (...) {
        return i_default;
    }
}

// return boolean value by primary key (run id, option key) or by option key only, return false if not found or value not "yes", "1", "true"
bool RunOptionTable::boolValue(bool i_useRunId, int i_runId, const char * i_key) const throw()
{
    try {
        string sVal = strValue(i_useRunId, i_runId, i_key);
        return 
            equalNoCase(sVal.c_str(), "1") || equalNoCase(sVal.c_str(), "yes") || equalNoCase(sVal.c_str(), "true");
    }
    catch (...) { 
        return false;
    }
}

// return long value by primary key (run id, option key) or by option key only, return default if not found or can not be converted to long
long long RunOptionTable::longValue(bool i_useRunId, int i_runId, const char * i_key, long long i_default) const throw()
{
    try {
        string sVal = strValue(i_useRunId, i_runId, i_key);
        return
            (sVal != "") ? std::stoll(sVal) : i_default;
    }
    catch (...) { 
        return i_default;
    }
}

// return double value by primary key (run id, option key) or by option key only, return default if not found or can not be converted to double
double RunOptionTable::doubleValue(bool i_useRunId, int i_runId, const char * i_key, double i_default) const throw()
{
    try {
        string sVal = strValue(i_useRunId, i_runId, i_key);
        return
            (sVal != "") ? std::stod(sVal) : i_default;
    }
    catch (...) { 
        return i_default;
    }
}
