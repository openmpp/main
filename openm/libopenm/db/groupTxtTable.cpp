// OpenM++ data library: group_txt table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // group_txt table implementation
    class GroupTxtTable : public IGroupTxtTable
    {
    public:
        GroupTxtTable(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);
        GroupTxtTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~GroupTxtTable() throw();

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by primary key: model id, group id, language id
        const GroupTxtRow * byKey(int i_modelId, int i_groupId, int i_langId) const;

        // get list of all table rows
        vector<GroupTxtRow> rows(void) const { return IMetaTable<GroupTxtRow>::rows(rowVec); }

        // get list of rows by language
        vector<GroupTxtRow> byLang(int i_langId) const;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        GroupTxtTable(const GroupTxtTable & i_table) = delete;
        GroupTxtTable & operator=(const GroupTxtTable & i_table) = delete;
    };

    // Columns type for group_txt row
    static const type_info * typeGroupTxtRow[] = { 
        &typeid(decltype(GroupTxtRow::modelId)), 
        &typeid(decltype(GroupTxtRow::groupId)), 
        &typeid(decltype(GroupTxtRow::langId)), 
        &typeid(decltype(GroupTxtRow::descr)), 
        &typeid(decltype(GroupTxtRow::note))
    };

    // Size (number of columns) for group_txt row
    static const int sizeGroupTxtRow = sizeof(typeGroupTxtRow) / sizeof(const type_info *);

    // Row adapter to select group_txt rows
    class GroupTxtRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new GroupTxtRow(); }
        int size(void) const { return sizeGroupTxtRow; }
        const type_info ** columnTypes(void) const { return typeGroupTxtRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<GroupTxtRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<GroupTxtRow *>(i_row)->groupId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<GroupTxtRow *>(i_row)->langId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<GroupTxtRow *>(i_row)->descr = ((const char *)i_value);
                break;
            case 4:
                dynamic_cast<GroupTxtRow *>(i_row)->note = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IGroupTxtTable::~IGroupTxtTable(void) throw() { }

// Create new table rows by loading db rows
IGroupTxtTable * IGroupTxtTable::create(IDbExec * i_dbExec, int i_modelId, int i_langId)
{
    return new GroupTxtTable(i_dbExec, i_modelId, i_langId);
}

// Create new table rows by swap with supplied vector of rows
IGroupTxtTable * IGroupTxtTable::create(IRowBaseVec & io_rowVec)
{
    return new GroupTxtTable(io_rowVec);
}

// Load table
GroupTxtTable::GroupTxtTable(IDbExec * i_dbExec, int i_modelId, int i_langId)   
{ 
    string sWhere = "";
    if (i_modelId > 0 && i_langId < 0) sWhere = " WHERE model_id = " + to_string(i_modelId);
    if (i_modelId <= 0 && i_langId >= 0) sWhere = " WHERE lang_id = " + to_string(i_langId);
    if (i_modelId > 0 && i_langId >= 0) sWhere = " WHERE model_id = " + to_string(i_modelId) + " AND lang_id = " + to_string(i_langId);

    const IRowAdapter & adp = GroupTxtRowAdapter();
    rowVec = IMetaTable<GroupTxtRow>::load(
        "SELECT model_id, group_id, lang_id, descr, note FROM group_txt" + sWhere + " ORDER BY 1, 2, 3", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
GroupTxtTable::~GroupTxtTable(void) throw() { }

// Find row by primary key: model id, group id, language id
const GroupTxtRow * GroupTxtTable::byKey(int i_modelId, int i_groupId, int i_langId) const
{
    const IRowBaseUptr keyRow( new GroupTxtRow(i_modelId, i_groupId, i_langId) );
    return IMetaTable<GroupTxtRow>::byKey(keyRow, rowVec);
}

// get list of rows by language
vector<GroupTxtRow> GroupTxtTable::byLang(int i_langId) const
{
    const IRowBaseUptr row( new GroupTxtRow(0, 0, i_langId) );
    return IMetaTable<GroupTxtRow>::findAll(row, rowVec, GroupTxtRow::langEqual);
}
