// OpenM++ data library: workset_lst table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // workset_lst table implementation
    class WorksetLstTable : public IWorksetLstTable
    {
    public:
        ~WorksetLstTable() throw();

        // select table rows by specified filter, sorted by primary key: set id
        static vector<WorksetLstRow> select(IDbExec * i_dbExec, const string & i_where);

    private:
        WorksetLstTable(const WorksetLstTable & i_table);               // = delete;
        WorksetLstTable & operator=(const WorksetLstTable & i_table);   // = delete;
    };

    // Columns type for workset_lst row
    static const type_info * typeWorksetLstRow[] = { 
        &typeid(decltype(WorksetLstRow::setId)), 
        &typeid(decltype(WorksetLstRow::runId)), 
        &typeid(decltype(WorksetLstRow::modelId)), 
        &typeid(decltype(WorksetLstRow::name)),
        &typeid(decltype(WorksetLstRow::isReadonly)),
        &typeid(decltype(WorksetLstRow::updateDateTime))
    };

    // Size (number of columns) for workset_lst row
    static const int sizeWorksetLstRow = sizeof(typeWorksetLstRow) / sizeof(const type_info *);

    // Row adapter to select workset_lst rows
    class WorksetLstRowAdapter : public IRowAdapter
    {
    public:
        WorksetLstRowAdapter() { }

        IRowBase * createRow(void) const { return new WorksetLstRow(); }
        int size(void) const { return sizeWorksetLstRow; }
        const type_info ** columnTypes(void) const { return typeWorksetLstRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<WorksetLstRow *>(i_row)->setId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<WorksetLstRow *>(i_row)->runId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<WorksetLstRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<WorksetLstRow *>(i_row)->name = ((const char *)i_value);
                break;
            case 4:
                dynamic_cast<WorksetLstRow *>(i_row)->isReadonly = (*(bool *)i_value);
                break;
            case 5:
                dynamic_cast<WorksetLstRow *>(i_row)->updateDateTime = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IWorksetLstTable::~IWorksetLstTable(void) throw() { }

// Table never unloaded
WorksetLstTable::~WorksetLstTable(void) throw() { }

// select table rows
vector<WorksetLstRow> IWorksetLstTable::select(IDbExec * i_dbExec, int i_modelId)
{
    string sWhere = (i_modelId > 0) ? " WHERE model_id = " + to_string(i_modelId) : "";
    return 
        WorksetLstTable::select(i_dbExec, sWhere);
}

// select table rows by set id
vector<WorksetLstRow> IWorksetLstTable::byKey(IDbExec * i_dbExec, int i_setId)
{
    string sWhere = " WHERE set_id = " + to_string(i_setId);
    return 
        WorksetLstTable::select(i_dbExec, sWhere);
}

// select table rows by specified filter
vector<WorksetLstRow> WorksetLstTable::select(IDbExec * i_dbExec, const string & i_where)
{
    if (i_dbExec == NULL) throw DbException("invalid (NULL) database connection");

    const IRowAdapter & adp = WorksetLstRowAdapter();
    IRowBaseVec vec = i_dbExec->selectRowList(
        "SELECT set_id, run_id, model_id, set_name, is_readonly, update_dt FROM workset_lst " + i_where + " ORDER BY 1", 
        adp
        );
    stable_sort(vec.begin(), vec.end(), WorksetLstRow::keyLess);

    return IMetaTable<WorksetLstRow>::rows(vec);
}
