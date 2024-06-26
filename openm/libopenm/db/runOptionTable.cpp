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
        ~RunOptionTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec& rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) override { return rowVec; }

        // find row by primary key: run id, option key
        const RunOptionRow * byKey(int i_runId, const string & i_key) const override;

        /** return true if primary key (run id, option key) found. */
        bool isExist(int i_runId, const char * i_key) const noexcept override;

        /** return string value by primary key (run id, option key) or default value if not found. */
        string strValue(int i_runId, const char * i_key, const string & i_default = "") const noexcept override;

        /** return boolean value by primary key (run id, option key) or false if not found or value not "yes", "1", "true" or empty "" string. */
        bool boolValue(int i_runId, const char * i_key) const noexcept override { return boolValueToInt(i_runId, i_key) > 0; };

        /** search for boolean value by primary key (run id, option key) and return one of: \n
        * return  1 if key found and value is one of: "yes", "1", "true" or empty value,    \n
        * return  0 if key found and value is one of: "no", "0", "false",                   \n
        * return -1 if key not found,                                                       \n
        * return -2 otherwise.
        */
        int boolValueToInt(int i_runId, const char * i_key) const noexcept override;

        /** return int value by primary key (run id, option key) or default if not found or can not be converted to int. */
        int intValue(int i_runId, const char * i_key, int i_default) const noexcept override;

        /** return long value by primary key (run id, option key) or default if not found or can not be converted to long. */
        long long longValue(int i_runId, const char * i_key, long long i_default) const noexcept override;

        /** return double value by primary key (run id, option key) or default if not found or can not be converted to double. */
        double doubleValue(int i_runId, const char * i_key, double i_default) const noexcept override;

    private:
        IRowBaseVec rowVec;     // table rows

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
        const type_info * const * columnTypes(void) const { return typeRunOptionRow; }

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
IRunOptionTable::~IRunOptionTable(void) noexcept { }

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
    rowVec = load(
        "SELECT run_id, option_key, option_value FROM run_option" +
        ((i_runId > 0) ? " WHERE run_id = " + to_string(i_runId) : "") +
        " ORDER BY 1, 2", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
RunOptionTable::~RunOptionTable(void) noexcept { }

// Find row by primary key: run id, option key
const RunOptionRow * RunOptionTable::byKey(int i_runId, const string & i_key) const
{
    const IRowBaseUptr keyRow( new RunOptionRow(i_runId, i_key) );
    return findKey(keyRow);
}

/** return true if primary key (run id, option key) found. */
bool RunOptionTable::isExist(int i_runId, const char * i_key) const noexcept
{ 
    try {
        return i_key != NULL && byKey(i_runId, i_key) != NULL;
    }
    catch (...) { 
        return false;
    }
}

// return string value by primary key (run id, option key) or by option key only, return default value if not found
string RunOptionTable::strValue(int i_runId, const char * i_key, const string & i_default) const noexcept
{
    try {
        if (i_key == NULL) return i_default;    // invalid key

        const RunOptionRow * optRow = byKey(i_runId, i_key);
        return
            (optRow != NULL) ? optRow->value : i_default;
    }
    catch (...) {
        return i_default;
    }
}

// search for boolean value by primary key (run id, option key) or by option key only and return one of:
// return  1 if key found and value is one of: "yes", "1", "true" or empty value,
// return  0 if key found and value is one of: "no", "0", "false",
// return -1 if key not found,
// return -2 otherwise.
int RunOptionTable::boolValueToInt(int i_runId, const char * i_key) const noexcept
{
    try {
        return isExist(i_runId, i_key) ? boolStringToInt(strValue(i_runId, i_key).c_str()) : -1;
    }
    catch (...) {
        return -2;
    }
}

// return int value by primary key (run id, option key) or by option key only, return default if not found or can not be converted to int
int RunOptionTable::intValue(int i_runId, const char * i_key, int i_default) const noexcept
{
    try {
        string sVal = strValue(i_runId, i_key);
        return
            (sVal != "") ? std::stoi(sVal) : i_default;
    }
    catch (...) { 
        return i_default;
    }
}

// return long value by primary key (run id, option key) or by option key only, return default if not found or can not be converted to long
long long RunOptionTable::longValue(int i_runId, const char * i_key, long long i_default) const noexcept
{
    try {
        string sVal = strValue(i_runId, i_key);
        return
            (sVal != "") ? std::stoll(sVal) : i_default;
    }
    catch (...) {
        return i_default;
    }
}

// return double value by primary key (run id, option key) or by option key only, return default if not found or can not be converted to double
double RunOptionTable::doubleValue(int i_runId, const char * i_key, double i_default) const noexcept
{
    try {
        string sVal = strValue(i_runId, i_key);
        return
            (sVal != "") ? std::stod(sVal) : i_default;
    }
    catch (...) { 
        return i_default;
    }
}
