// OpenM++ data library: run_lst table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // run_lst table implementation
    class RunLstTable : public IRunLstTable
    {
    public:
        ~RunLstTable() throw();

        // select table rows by specified filter, sorted by primary key: run id
        static vector<RunLstRow> select(IDbExec * i_dbExec, const string & i_where);

    private:
        RunLstTable(const RunLstTable & i_table);               // = delete;
        RunLstTable & operator=(const RunLstTable & i_table);   // = delete;
    };

    // Columns type for run_lst row
    static const type_info * typeRunLstRow[] = { 
        &typeid(decltype(RunLstRow::runId)), 
        &typeid(decltype(RunLstRow::modelId)), 
        &typeid(decltype(RunLstRow::name)), 
        &typeid(decltype(RunLstRow::subCount)),
        &typeid(decltype(RunLstRow::subStarted)),
        &typeid(decltype(RunLstRow::subCompleted)),
        &typeid(decltype(RunLstRow::createDateTime)),
        &typeid(decltype(RunLstRow::status)),
        &typeid(decltype(RunLstRow::updateDateTime))
    };

    // Size (number of columns) for run_lst row
    static const int sizeRunLstRow = sizeof(typeRunLstRow) / sizeof(const type_info *);

    // Row adapter to select run_lst rows
    class RunLstRowAdapter : public IRowAdapter
    {
    public:
        RunLstRowAdapter() { }

        IRowBase * createRow(void) const { return new RunLstRow(); }
        int size(void) const { return sizeRunLstRow; }
        const type_info ** columnTypes(void) const { return typeRunLstRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<RunLstRow *>(i_row)->runId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<RunLstRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<RunLstRow *>(i_row)->name = ((const char *)i_value);
                break;
            case 3:
                dynamic_cast<RunLstRow *>(i_row)->subCount = (*(int *)i_value);
                break;
            case 4:
                dynamic_cast<RunLstRow *>(i_row)->subStarted = (*(int *)i_value);
                break;
            case 5:
                dynamic_cast<RunLstRow *>(i_row)->subCompleted = (*(int *)i_value);
                break;
            case 6:
                dynamic_cast<RunLstRow *>(i_row)->createDateTime = ((const char *)i_value);
                break;
            case 7:
                dynamic_cast<RunLstRow *>(i_row)->status = ((const char *)i_value);
                break;
            case 8:
                dynamic_cast<RunLstRow *>(i_row)->updateDateTime = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IRunLstTable::~IRunLstTable(void) throw() { }

// Table never unloaded
RunLstTable::~RunLstTable(void) throw() { }

// select table rows
vector<RunLstRow> IRunLstTable::select(IDbExec * i_dbExec, int i_modelId)
{
    string sWhere = (i_modelId > 0) ? " WHERE model_id = " + to_string(i_modelId) : "";
    return 
        RunLstTable::select(i_dbExec, sWhere);
}

// select table rows by run id
vector<RunLstRow> IRunLstTable::byKey(IDbExec * i_dbExec, int i_runId)
{
    string sWhere = " WHERE run_id = " + to_string(i_runId);
    return 
        RunLstTable::select(i_dbExec, sWhere);
}

// select table rows by specified filter
vector<RunLstRow> RunLstTable::select(IDbExec * i_dbExec, const string & i_where)
{
    if (i_dbExec == NULL) throw DbException("invalid (NULL) database connection");

    const IRowAdapter & adp = RunLstRowAdapter();
    IRowBaseVec vec = i_dbExec->selectRowList(
        "SELECT run_id, model_id, run_name, sub_count, sub_started, sub_completed, create_dt, status, update_dt" \
        " FROM run_lst " + i_where + " ORDER BY 1", 
        adp
        );
    stable_sort(vec.begin(), vec.end(), RunLstRow::keyLess);

    return IMetaTable<RunLstRow>::rows(vec);
}
