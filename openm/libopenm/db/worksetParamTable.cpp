// OpenM++ data library: workset_parameter join to model_parameter_dic table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // workset_parameter join to model_parameter_dic table implementation
    class WorksetParamTable : public IWorksetParamTable
    {
    public:
        ~WorksetParamTable() noexcept;

        // select table rows by specified filter, sorted by primary key: set id, parameter id
        static vector<WorksetParamRow> select(IDbExec * i_dbExec, const string & i_where);

    private:
        WorksetParamTable(const WorksetParamTable & i_table) = delete;
        WorksetParamTable & operator=(const WorksetParamTable & i_table) = delete;
    };

    // Columns type for workset_parameter row
    static const type_info * typeWorksetParamRow[] = { 
        &typeid(decltype(WorksetParamRow::setId)), 
        &typeid(decltype(WorksetParamRow::modelId)), 
        &typeid(decltype(WorksetParamRow::paramId)),
        &typeid(decltype(WorksetParamRow::subCount)),
        &typeid(decltype(WorksetParamRow::defaultSubId))
    };

    // Size (number of columns) for workset_parameter join to model_parameter_dic row
    static const int sizeWorksetParamRow = sizeof(typeWorksetParamRow) / sizeof(const type_info *);

    // Row adapter to select workset_parameter join to model_parameter_dic rows
    class WorksetParamRowAdapter : public IRowAdapter
    {
    public:
        WorksetParamRowAdapter() { }

        IRowBase * createRow(void) const { return new WorksetParamRow(); }
        int size(void) const { return sizeWorksetParamRow; }
        const type_info * const * columnTypes(void) const { return typeWorksetParamRow; }

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
            case 3:
                dynamic_cast<WorksetParamRow *>(i_row)->subCount = (*(int *)i_value);
                break;
            case 4:
                dynamic_cast<WorksetParamRow *>(i_row)->defaultSubId = (*(int *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IWorksetParamTable::~IWorksetParamTable(void) noexcept { }

// Table never unloaded
WorksetParamTable::~WorksetParamTable(void) noexcept { }

// select table rows
vector<WorksetParamRow> IWorksetParamTable::select(IDbExec * i_dbExec, int i_setId)
{
    string sWhere = (i_setId > 0) ? " WHERE W.set_id = " + to_string(i_setId) : "";
    return 
        WorksetParamTable::select(i_dbExec, sWhere);
}

// select table rows by set id and parameter id
vector<WorksetParamRow> IWorksetParamTable::byKey(IDbExec * i_dbExec, int i_setId, int i_paramId)
{
    string sWhere = " WHERE W.set_id = " + to_string(i_setId) + " AND MP.model_parameter_id = " + to_string(i_paramId);
    return 
        WorksetParamTable::select(i_dbExec, sWhere);
}

// select table rows by specified filter
vector<WorksetParamRow> WorksetParamTable::select(IDbExec * i_dbExec, const string & i_where)
{
    if (i_dbExec == NULL) throw DbException("invalid (NULL) database connection");

    const IRowAdapter & adp = WorksetParamRowAdapter();
    IRowBaseVec vec = i_dbExec->selectRowVector(
        "SELECT W.set_id, MP.model_id, MP.model_parameter_id, WP.sub_count, WP.default_sub_id" \
        " FROM workset_lst W" \
        " INNER JOIN workset_parameter WP ON (WP.set_id = W.set_id)" \
        " INNER JOIN model_parameter_dic MP ON (MP.model_id = W.model_id AND MP.parameter_hid = WP.parameter_hid)" +
        i_where + 
        " ORDER BY 1, 3", 
        adp
        );
    stable_sort(vec.begin(), vec.end(), WorksetParamRow::keyLess);

    return IMetaTable<WorksetParamRow>::rows(vec);
}
