// OpenM++ data library: run_parameter table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // run_parameter table implementation
    class RunParamTable : public IRunParamTable
    {
    public:
        ~RunParamTable() throw();

        // select table rows by specified filter, sorted by primary key: run id
        static vector<RunParamRow> select(IDbExec * i_dbExec, const string & i_where);

    private:
        RunParamTable(const RunParamTable & i_table) = delete;
        RunParamTable & operator=(const RunParamTable & i_table) = delete;
    };

    // Columns type for run_parameter row
    static const type_info * typeRunParamRow[] = { 
        &typeid(decltype(RunParamRow::runId)), 
        &typeid(decltype(RunParamRow::modelId)), 
        &typeid(decltype(RunParamRow::paramId)), 
        &typeid(decltype(RunParamRow::baseRunId))
    };

    // Size (number of columns) for run_parameter row
    static const int sizeRunParamRow = sizeof(typeRunParamRow) / sizeof(const type_info *);

    // Row adapter to select run_parameter rows
    class RunParamRowAdapter : public IRowAdapter
    {
    public:
        RunParamRowAdapter() { }

        IRowBase * createRow(void) const { return new RunParamRow(); }
        int size(void) const { return sizeRunParamRow; }
        const type_info ** columnTypes(void) const { return typeRunParamRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<RunParamRow *>(i_row)->runId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<RunParamRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<RunParamRow *>(i_row)->paramId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<RunParamRow *>(i_row)->baseRunId = (*(int *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IRunParamTable::~IRunParamTable(void) throw() { }

// Table never unloaded
RunParamTable::~RunParamTable(void) throw() { }

// select table rows by run id
vector<RunParamRow> IRunParamTable::select(IDbExec * i_dbExec, int i_runId)
{
    string sWhere = " WHERE run_id = " + to_string(i_runId);
    return 
        RunParamTable::select(i_dbExec, sWhere);
}

// select table rows by run id, parameter id
vector<RunParamRow> IRunParamTable::byKey(IDbExec * i_dbExec, int i_runId, int i_paramId)
{
    string sWhere = " WHERE run_id = " + to_string(i_runId) + " AND parameter_id = " + to_string(i_paramId);
    return 
        RunParamTable::select(i_dbExec, sWhere);
}

// select table rows by specified filter
vector<RunParamRow> RunParamTable::select(IDbExec * i_dbExec, const string & i_where)
{
    if (i_dbExec == NULL) throw DbException("invalid (NULL) database connection");

    const IRowAdapter & adp = RunParamRowAdapter();
    IRowBaseVec vec = i_dbExec->selectRowList(
        "SELECT run_id, model_id, parameter_id, base_run_id" \
        " FROM run_parameter " + i_where + " ORDER BY 1, 3", 
        adp
        );
    stable_sort(vec.begin(), vec.end(), RunParamRow::keyLess);

    return IMetaTable<RunParamRow>::rows(vec);
}

// select base run id by run id and parameter id, return zero if not exist
int IRunParamTable::selectBaseRunId(IDbExec * i_dbExec, int i_runId, int i_paramId)
{
    if (i_dbExec == NULL) throw DbException("invalid (NULL) database connection");

    return i_dbExec->selectToInt(
        "SELECT base_run_id FROM run_parameter " \
        " WHERE run_id = " + to_string(i_runId) + " AND parameter_id = " + to_string(i_paramId),
        0);
}
