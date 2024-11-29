// OpenM++ data library: entity_group_pc table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // entity_group_pc table implementation
    class EntityGroupPcTable : public IEntityGroupPcTable
    {
    public:
        EntityGroupPcTable(IDbExec * i_dbExec, int i_modelId = 0);
        EntityGroupPcTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~EntityGroupPcTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec & rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) override { return rowVec; }

        // find row by primary key: model id, entity id, group id, child position
        const EntityGroupPcRow * byKey(int i_modelId, int i_entityId, int i_groupId, int i_chidPos) const override;

        // get list of rows by model id
        vector<EntityGroupPcRow> byModelId(int i_modelId) const override;

        // get list of attribute id's for the group: list of bottom level group members.
        vector<int> groupAttrs(int i_modelId, int i_groupId) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        EntityGroupPcTable(const EntityGroupPcTable & i_table) = delete;
        EntityGroupPcTable & operator=(const EntityGroupPcTable & i_table) = delete;
    };

    // Columns type for entity_group_pc row
    static const type_info * typeEntityGroupPcRow[] = { 
        &typeid(decltype(EntityGroupPcRow::modelId)), 
        &typeid(decltype(EntityGroupPcRow::entityId)), 
        &typeid(decltype(EntityGroupPcRow::groupId)), 
        &typeid(decltype(EntityGroupPcRow::childPos)), 
        &typeid(decltype(EntityGroupPcRow::childGroupId)), 
        &typeid(decltype(EntityGroupPcRow::attrId))
    };

    // Size (number of columns) for entity_group_pc row
    static const int sizeEntityGroupPcRow = sizeof(typeEntityGroupPcRow) / sizeof(const type_info *);

    // Row adapter to select entity_group_pc rows
    class EntityGroupPcRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new EntityGroupPcRow(); }
        int size(void) const { return sizeEntityGroupPcRow; }
        const type_info * const * columnTypes(void) const { return typeEntityGroupPcRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<EntityGroupPcRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<EntityGroupPcRow *>(i_row)->entityId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<EntityGroupPcRow *>(i_row)->groupId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<EntityGroupPcRow *>(i_row)->childPos = (*(int *)i_value);
                break;
            case 4:
                dynamic_cast<EntityGroupPcRow *>(i_row)->childGroupId = (*(int *)i_value);
                break;
            case 5:
                dynamic_cast<EntityGroupPcRow *>(i_row)->attrId = (*(int *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IEntityGroupPcTable::~IEntityGroupPcTable(void) noexcept { }

// Create new table rows by loading db rows
IEntityGroupPcTable * IEntityGroupPcTable::create(IDbExec * i_dbExec, int i_modelId)
{
    return new EntityGroupPcTable(i_dbExec, i_modelId);
}

// Create new table rows by swap with supplied vector of rows
IEntityGroupPcTable * IEntityGroupPcTable::create(IRowBaseVec & io_rowVec)
{
    return new EntityGroupPcTable(io_rowVec);
}

// Load table
EntityGroupPcTable::EntityGroupPcTable(IDbExec * i_dbExec, int i_modelId)
{ 
    const IRowAdapter & adp = EntityGroupPcRowAdapter();
    rowVec = load(
        "SELECT" \
        " model_id, model_entity_id, group_id, child_pos, child_group_id, attr_id" \
        " FROM entity_group_pc" +
        ((i_modelId > 0) ? " WHERE model_id = " + to_string(i_modelId) : "") +
        " ORDER BY 1, 2, 3, 4", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
EntityGroupPcTable::~EntityGroupPcTable(void) noexcept { }

// Find row by primary key: model id, entity id, group id, child position
const EntityGroupPcRow * EntityGroupPcTable::byKey(int i_modelId, int i_entityId, int i_groupId, int i_chidPos) const
{
    const IRowBaseUptr keyRow( new EntityGroupPcRow(i_modelId, i_entityId, i_groupId, i_chidPos) );
    return findKey(keyRow);
}

// get list of rows by model id
vector<EntityGroupPcRow> EntityGroupPcTable::byModelId(int i_modelId) const
{
    return findAll(
        [i_modelId](const EntityGroupPcRow & i_row) -> bool { return i_row.modelId == i_modelId; }
    );
}

// get list of attribute id's for the group: list of bottom level group members.
vector<int> EntityGroupPcTable::groupAttrs(int i_modelId, int i_groupId) const
{
    unordered_map<int, bool> groups;    // group id and flag to mark already processed groups
    set<int> attrIds;                   // leafs: attribute id's of the group

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

        // add all child groups of current group into group list, add all leafs into attributes set
        for (auto it = rowVec.cbegin(); it != rowVec.cend(); ++it) {
            EntityGroupPcRow * grp = dynamic_cast<EntityGroupPcRow *>(it->get());

            if (grp->modelId != i_modelId || grp->groupId != grpIt->first) continue;    // not a child of current group

            // if this is a leaf then add attribute id into results
            if (grp->attrId >= 0) {
                attrIds.insert(grp->attrId);
            }
            // if this is a child group add it to groups list if not already there
            if (grp->childGroupId >= 0) {
                if (groups.find(grp->childGroupId) == groups.end()) groups.insert({ {grp->childGroupId, false} });
            }
        }
    }

    return vector<int>(attrIds.cbegin(), attrIds.cend());
}
