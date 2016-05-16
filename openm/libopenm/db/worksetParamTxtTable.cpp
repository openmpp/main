// OpenM++ data library: workset_parameter_txt join to model_parameter_dic table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // workset_parameter_txt join to model_parameter_dic table implementation
    class WorksetParamTxtTable : public IWorksetParamTxtTable
    {
    public:
        ~WorksetParamTxtTable() throw();

        // select table rows by specified filter, sorted by primary key: set id, parameter id, language id
        static vector<WorksetParamTxtRow> select(IDbExec * i_dbExec, const string & i_where);

    private:
        WorksetParamTxtTable(const WorksetParamTxtTable & i_table) = delete;
        WorksetParamTxtTable & operator=(const WorksetParamTxtTable & i_table) = delete;
    };

    // Columns type for workset_parameter_txt join to model_parameter_dic row
    static const type_info * typeWorksetParamTxtRow[] = { 
        &typeid(decltype(WorksetParamTxtRow::setId)), 
        &typeid(decltype(WorksetParamTxtRow::modelId)), 
        &typeid(decltype(WorksetParamTxtRow::paramId)), 
        &typeid(decltype(WorksetParamTxtRow::langId)), 
        &typeid(decltype(WorksetParamTxtRow::note))
    };

    // Size (number of columns) for workset_parameter_txt join to model_parameter_dic row
    static const int sizeWorksetParamTxtRow = sizeof(typeWorksetParamTxtRow) / sizeof(const type_info *);

    // Row adapter to select workset_parameter_txt rows
    class WorksetParamTxtRowAdapter : public IRowAdapter
    {
    public:
        WorksetParamTxtRowAdapter() { }

        IRowBase * createRow(void) const { return new WorksetParamTxtRow(); }
        int size(void) const { return sizeWorksetParamTxtRow; }
        const type_info * const * columnTypes(void) const { return typeWorksetParamTxtRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<WorksetParamTxtRow *>(i_row)->setId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<WorksetParamTxtRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<WorksetParamTxtRow *>(i_row)->paramId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<WorksetParamTxtRow *>(i_row)->langId = (*(int *)i_value);
                break;
            case 4:
                dynamic_cast<WorksetParamTxtRow *>(i_row)->note = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IWorksetParamTxtTable::~IWorksetParamTxtTable(void) throw() { }

// Table never unloaded
WorksetParamTxtTable::~WorksetParamTxtTable(void) throw() { }

// select table rows
vector<WorksetParamTxtRow> IWorksetParamTxtTable::select(IDbExec * i_dbExec, int i_setId, int i_langId)
{
    string sWhere = "";
    if (i_setId > 0 && i_langId < 0) sWhere = " WHERE T.set_id = " + to_string(i_setId);
    if (i_setId <= 0 && i_langId >= 0) sWhere = " WHERE T.lang_id = " + to_string(i_langId);
    if (i_setId > 0 && i_langId >= 0) sWhere = " WHERE T.set_id = " + to_string(i_setId) + " AND T.lang_id = " + to_string(i_langId);

    return 
        WorksetParamTxtTable::select(i_dbExec, sWhere);
}

// select table rows by set id, parameter id, language id
vector<WorksetParamTxtRow> IWorksetParamTxtTable::byKey(IDbExec * i_dbExec, int i_setId, int i_paramId, int i_langId)
{
    string sWhere = 
        " WHERE T.set_id = " + to_string(i_setId) + " AND M.model_parameter_id = " + to_string(i_paramId) + " AND T.lang_id = " + to_string(i_langId);
    return 
        WorksetParamTxtTable::select(i_dbExec, sWhere);
}

// select table rows by specified filter
vector<WorksetParamTxtRow> WorksetParamTxtTable::select(IDbExec * i_dbExec, const string & i_where)
{
    if (i_dbExec == NULL) throw DbException("invalid (NULL) database connection");

    const IRowAdapter & adp = WorksetParamTxtRowAdapter();
    IRowBaseVec vec = i_dbExec->selectRowVector(
        "SELECT T.set_id, M.model_id, M.model_parameter_id, T.lang_id, T.note" \
        " FROM workset_parameter_txt T" \
        " INNER JOIN model_parameter_dic M ON (M.parameter_hid = T.parameter_hid)" +
        i_where + 
        " ORDER BY 1, 3, 4", 
        adp
        );
    stable_sort(vec.begin(), vec.end(), WorksetParamTxtRow::keyLess);

    return IMetaTable<WorksetParamTxtRow>::rows(vec);
}
