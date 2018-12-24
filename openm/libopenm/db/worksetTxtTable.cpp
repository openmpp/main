// OpenM++ data library: workset_txt table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // workset_txt table implementation
    class WorksetTxtTable : public IWorksetTxtTable
    {
    public:
        ~WorksetTxtTable() noexcept;

        // select table rows by specified filter, sorted by primary key: set id and language id
        static vector<WorksetTxtRow> select(IDbExec * i_dbExec, const string & i_where);

    private:
        WorksetTxtTable(const WorksetTxtTable & i_table) = delete;
        WorksetTxtTable & operator=(const WorksetTxtTable & i_table) = delete;
    };

    // Columns type for workset_txt row
    static const type_info * typeWorksetTxtRow[] = { 
        &typeid(decltype(WorksetTxtRow::setId)), 
        &typeid(decltype(WorksetTxtRow::langId)), 
        &typeid(decltype(WorksetTxtRow::descr)),
        &typeid(decltype(WorksetTxtRow::note))
    };

    // Size (number of columns) for workset_txt row
    static const int sizeWorksetTxtRow = sizeof(typeWorksetTxtRow) / sizeof(const type_info *);

    // Row adapter to select workset_txt rows
    class WorksetTxtRowAdapter : public IRowAdapter
    {
    public:
        WorksetTxtRowAdapter() { }

        IRowBase * createRow(void) const { return new WorksetTxtRow(); }
        int size(void) const { return sizeWorksetTxtRow; }
        const type_info * const * columnTypes(void) const { return typeWorksetTxtRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<WorksetTxtRow *>(i_row)->setId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<WorksetTxtRow *>(i_row)->langId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<WorksetTxtRow *>(i_row)->descr = ((const char *)i_value);
                break;
            case 3:
                dynamic_cast<WorksetTxtRow *>(i_row)->note = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IWorksetTxtTable::~IWorksetTxtTable(void) noexcept { }

// Table never unloaded
WorksetTxtTable::~WorksetTxtTable(void) noexcept { }

// select table rows
vector<WorksetTxtRow> IWorksetTxtTable::select(IDbExec * i_dbExec, int i_langId)
{
    string sWhere = "";
    if (i_langId >= 0) sWhere = " WHERE lang_id = " + to_string(i_langId);

    return 
        WorksetTxtTable::select(i_dbExec, sWhere);
}

// select table rows by set id and language id
vector<WorksetTxtRow> IWorksetTxtTable::byKey(IDbExec * i_dbExec, int i_setId, int i_langId)
{
    string sWhere = " WHERE set_id = " + to_string(i_setId) + " AND lang_id = " + to_string(i_langId);
    return 
        WorksetTxtTable::select(i_dbExec, sWhere);
}

// select table rows by specified filter
vector<WorksetTxtRow> WorksetTxtTable::select(IDbExec * i_dbExec, const string & i_where)
{
    if (i_dbExec == NULL) throw DbException("invalid (NULL) database connection");

    const IRowAdapter & adp = WorksetTxtRowAdapter();
    IRowBaseVec vec = i_dbExec->selectRowVector(
        "SELECT set_id, lang_id, descr, note FROM workset_txt " + i_where + " ORDER BY 1, 2", 
        adp
        );
    stable_sort(vec.begin(), vec.end(), WorksetTxtRow::keyLess);

    return IMetaTable<WorksetTxtRow>::rows(vec);
}
