// OpenM++ data library: workset_parameter table
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // workset_parameter table implementation
    class WorksetParamTable : public IWorksetParamTable
    {
    public:
        ~WorksetParamTable() throw();

        // select table rows by specified filter, sorted by primary key: set id, parameter id
        static vector<WorksetParamRow> select(IDbExec * i_dbExec, const string & i_where);

    private:
        WorksetParamTable(const WorksetParamTable & i_table);               // = delete;
        WorksetParamTable & operator=(const WorksetParamTable & i_table);   // = delete;
    };

    // Columns type for workset_parameter row
    static const type_info * typeWorksetParamRow[] = { 
        &typeid(decltype(WorksetParamRow::setId)), 
        &typeid(decltype(WorksetParamRow::modelId)), 
        &typeid(decltype(WorksetParamRow::paramId))
    };

    // Size (number of columns) for workset_parameter row
    static const int sizeWorksetParamRow = sizeof(typeWorksetParamRow) / sizeof(const type_info *);

    // Row adapter to select workset_parameter rows
    class WorksetParamRowAdapter : public IRowAdapter
    {
    public:
        WorksetParamRowAdapter() { }

        IRowBase * createRow(void) const { return new WorksetParamRow(); }
        int size(void) const { return sizeWorksetParamRow; }
        const type_info ** columnTypes(void) const { return typeWorksetParamRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<WorksetParamRow *>(i_row)->setId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<WorksetParamRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<WorksetParamRow *>(i_row)->paramId = (*(int *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IWorksetParamTable::~IWorksetParamTable(void) throw() { }

// Table never unloaded
WorksetParamTable::~WorksetParamTable(void) throw() { }

// select table rows
vector<WorksetParamRow> IWorksetParamTable::select(IDbExec * i_dbExec, int i_setId)
{
    string sWhere = (i_setId > 0) ? " WHERE set_id = " + to_string(i_setId) : "";
    return 
        WorksetParamTable::select(i_dbExec, sWhere);
}

// select table rows by set id and parameter id
vector<WorksetParamRow> IWorksetParamTable::byKey(IDbExec * i_dbExec, int i_setId, int i_paramId)
{
    string sWhere = " WHERE set_id = " + to_string(i_setId) + " AND parameter_id = " + to_string(i_paramId);
    return 
        WorksetParamTable::select(i_dbExec, sWhere);
}

// select table rows by specified filter
vector<WorksetParamRow> WorksetParamTable::select(IDbExec * i_dbExec, const string & i_where)
{
    if (i_dbExec == NULL) throw DbException("invalid (NULL) database connection");

    const IRowAdapter & adp = WorksetParamRowAdapter();
    IRowBaseVec vec = i_dbExec->selectRowList(
        "SELECT set_id, model_id, parameter_id FROM workset_parameter " + i_where + " ORDER BY 1, 3", 
        adp
        );
    stable_sort(vec.begin(), vec.end(), WorksetParamRow::keyLess);

    return IMetaTable<WorksetParamRow>::rows(vec);
}

