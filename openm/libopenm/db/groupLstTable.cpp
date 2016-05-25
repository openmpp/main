// OpenM++ data library: group_lst table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // group_lst table implementation
    class GroupLstTable : public IGroupLstTable
    {
    public:
        GroupLstTable(IDbExec * i_dbExec, int i_modelId = 0);
        GroupLstTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~GroupLstTable() throw();

        // get const reference to list of all table rows
        const IRowBaseVec & rowsCRef(void) const { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by primary key: model id and group id
        const GroupLstRow * byKey(int i_modelId, int i_groupId) const;

        // get list of rows by model id and is parameter group flag
        vector<GroupLstRow> byModelId(int i_modelId, bool i_isParam) const;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        GroupLstTable(const GroupLstTable & i_table) = delete;
        GroupLstTable & operator=(const GroupLstTable & i_table) = delete;
    };

    // Columns type for group_lst row
    static const type_info * typeGroupLstRow[] = { 
        &typeid(decltype(GroupLstRow::modelId)), 
        &typeid(decltype(GroupLstRow::groupId)), 
        &typeid(decltype(GroupLstRow::isParam)), 
        &typeid(decltype(GroupLstRow::name)), 
        &typeid(decltype(GroupLstRow::isHidden)), 
        &typeid(decltype(GroupLstRow::isGenerated))
    };

    // Size (number of columns) for group_lst row
    static const int sizeGroupLstRow = sizeof(typeGroupLstRow) / sizeof(const type_info *);

    // Row adapter to select group_lst rows
    class GroupLstRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new GroupLstRow(); }
        int size(void) const { return sizeGroupLstRow; }
        const type_info * const * columnTypes(void) const { return typeGroupLstRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<GroupLstRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<GroupLstRow *>(i_row)->groupId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<GroupLstRow *>(i_row)->isParam = (*(bool *)i_value);
                break;
            case 3:
                dynamic_cast<GroupLstRow *>(i_row)->name = ((const char *)i_value);
                break;
            case 4:
                dynamic_cast<GroupLstRow *>(i_row)->isHidden = (*(bool *)i_value);
                break;
            case 5:
                dynamic_cast<GroupLstRow *>(i_row)->isGenerated = (*(bool *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IGroupLstTable::~IGroupLstTable(void) throw() { }

// Create new table rows by loading db rows
IGroupLstTable * IGroupLstTable::create(IDbExec * i_dbExec, int i_modelId)
{
    return new GroupLstTable(i_dbExec, i_modelId);
}

// Create new table rows by swap with supplied vector of rows
IGroupLstTable * IGroupLstTable::create(IRowBaseVec & io_rowVec)
{
    return new GroupLstTable(io_rowVec);
}

// Load table
GroupLstTable::GroupLstTable(IDbExec * i_dbExec, int i_modelId)
{ 
    const IRowAdapter & adp = GroupLstRowAdapter();
    rowVec = load(
        "SELECT" \
        " model_id, group_id, is_parameter, group_name, is_hidden, is_generated" \
        " FROM group_lst" +
        ((i_modelId > 0) ? " WHERE model_id = " + to_string(i_modelId) : "") +
        " ORDER BY 1, 2", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
GroupLstTable::~GroupLstTable(void) throw() { }

// Find row by primary key: model id and group id
const GroupLstRow * GroupLstTable::byKey(int i_modelId, int i_groupId) const
{
    const IRowBaseUptr keyRow( new GroupLstRow(i_modelId, i_groupId) );
    return findKey(keyRow);
}

// get list of rows by model id and is parameter group flag
vector<GroupLstRow> GroupLstTable::byModelId(int i_modelId, bool i_isParam) const
{
    return findAll(
        [i_modelId, i_isParam](const GroupLstRow & i_row) -> bool {
                return i_row.modelId == i_modelId && i_row.isParam == i_isParam;
            }
        );
}
