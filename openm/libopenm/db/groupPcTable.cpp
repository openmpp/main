// OpenM++ data library: group_pc table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // group_pc table implementation
    class GroupPcTable : public IGroupPcTable
    {
    public:
        GroupPcTable(IDbExec * i_dbExec, int i_modelId = 0);
        GroupPcTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~GroupPcTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec & rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) override { return rowVec; }

        // find row by primary key: model id, group id, child position
        const GroupPcRow * byKey(int i_modelId, int i_groupId, int i_chidPos) const override;

        // get list of rows by model id
        vector<GroupPcRow> byModelId(int i_modelId) const override;

        // get list of parameter id's or table id's for the group: list of bottom level group members.
        vector<int> groupLeafs(int i_modelId, int i_groupId) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        GroupPcTable(const GroupPcTable & i_table) = delete;
        GroupPcTable & operator=(const GroupPcTable & i_table) = delete;
    };

    // Columns type for group_pc row
    static const type_info * typeGroupPcRow[] = { 
        &typeid(decltype(GroupPcRow::modelId)), 
        &typeid(decltype(GroupPcRow::groupId)), 
        &typeid(decltype(GroupPcRow::childPos)), 
        &typeid(decltype(GroupPcRow::childGroupId)), 
        &typeid(decltype(GroupPcRow::leafId))
    };

    // Size (number of columns) for group_pc row
    static const int sizeGroupPcRow = sizeof(typeGroupPcRow) / sizeof(const type_info *);

    // Row adapter to select group_pc rows
    class GroupPcRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new GroupPcRow(); }
        int size(void) const { return sizeGroupPcRow; }
        const type_info * const * columnTypes(void) const { return typeGroupPcRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<GroupPcRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<GroupPcRow *>(i_row)->groupId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<GroupPcRow *>(i_row)->childPos = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<GroupPcRow *>(i_row)->childGroupId = (*(int *)i_value);
                break;
            case 4:
                dynamic_cast<GroupPcRow *>(i_row)->leafId = (*(int *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IGroupPcTable::~IGroupPcTable(void) noexcept { }

// Create new table rows by loading db rows
IGroupPcTable * IGroupPcTable::create(IDbExec * i_dbExec, int i_modelId)
{
    return new GroupPcTable(i_dbExec, i_modelId);
}

// Create new table rows by swap with supplied vector of rows
IGroupPcTable * IGroupPcTable::create(IRowBaseVec & io_rowVec)
{
    return new GroupPcTable(io_rowVec);
}

// Load table
GroupPcTable::GroupPcTable(IDbExec * i_dbExec, int i_modelId)
{ 
    const IRowAdapter & adp = GroupPcRowAdapter();
    rowVec = load(
        "SELECT" \
        " model_id, group_id, child_pos, child_group_id, leaf_id" \
        " FROM group_pc" +
        ((i_modelId > 0) ? " WHERE model_id = " + to_string(i_modelId) : "") +
        " ORDER BY 1, 2, 3", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
GroupPcTable::~GroupPcTable(void) noexcept { }

// Find row by primary key: model id, group id, child position
const GroupPcRow * GroupPcTable::byKey(int i_modelId, int i_groupId, int i_chidPos) const
{
    const IRowBaseUptr keyRow( new GroupPcRow(i_modelId, i_groupId, i_chidPos) );
    return findKey(keyRow);
}

// get list of rows by model id
vector<GroupPcRow> GroupPcTable::byModelId(int i_modelId) const
{
    return findAll(
        [i_modelId](const GroupPcRow & i_row) -> bool { return i_row.modelId == i_modelId; }
    );
}

// get list of parameter id's or table id's for the group: list of bottom level group members.
vector<int> GroupPcTable::groupLeafs(int i_modelId, int i_groupId) const
{
    unordered_map<int, bool> groups;    // group id and flag to mark already processed groups
    set<int> leafIds;                   // leaf parameter id's or table id's

    // starting from source group process all child groups and add leafs id into result
    groups[i_groupId] = false;
    bool isAllDone = false;

    while (!isAllDone) {

        // find first new group: group which is not yet processed
        isAllDone = true;
        auto grpIt = groups.begin();

        for (; grpIt != groups.end(); ++grpIt) {
            isAllDone &= grpIt->second;
            if (!grpIt->second) break;  // that group is not processed yet
        }
        if (isAllDone) break;       // all groups already processed

        grpIt->second = true;       // mark current group as completed

        // add all child groups of current group into group list, add all leafs into leafs set
        for (auto it = rowVec.cbegin(); it != rowVec.cend(); ++it) {
            GroupPcRow * grp = dynamic_cast<GroupPcRow *>(it->get());

            if (grp->modelId != i_modelId || grp->groupId != grpIt->first) continue;    // not a child of current group

            // if this is a leaf then add leaf id into results
            if (grp->leafId >= 0) {
                leafIds.insert(grp->leafId);
            }
            // if this is a child group add it to groups list if not already there
            if (grp->childGroupId >= 0) {
                if (groups.find(grp->childGroupId) == groups.end()) groups.insert({ {grp->childGroupId, false} });
            }
        }
    }

    return vector<int>(leafIds.cbegin(), leafIds.cend());
}
