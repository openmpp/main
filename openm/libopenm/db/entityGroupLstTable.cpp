// OpenM++ data library: entity_group_lst table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // entity_group_lst table implementation
    class EntityGroupLstTable : public IEntityGroupLstTable
    {
    public:
        EntityGroupLstTable(IDbExec * i_dbExec, int i_modelId = 0);
        EntityGroupLstTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~EntityGroupLstTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec & rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) override { return rowVec; }

        // find row by primary key: model id, entity id, group id
        const EntityGroupLstRow * byKey(int i_modelId, int i_entityId, int i_groupId) const override;

        // get list of rows by model id
        vector<EntityGroupLstRow> byModelId(int i_modelId) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        EntityGroupLstTable(const EntityGroupLstTable & i_table) = delete;
        EntityGroupLstTable & operator=(const EntityGroupLstTable & i_table) = delete;
    };

    // Columns type for entity_group_lst row
    static const type_info * typeEntityGroupLstRow[] = { 
        &typeid(decltype(EntityGroupLstRow::modelId)), 
        &typeid(decltype(EntityGroupLstRow::entityId)), 
        &typeid(decltype(EntityGroupLstRow::groupId)), 
        &typeid(decltype(EntityGroupLstRow::name)), 
        &typeid(decltype(EntityGroupLstRow::isHidden))
    };

    // Size (number of columns) for entity_group_lst row
    static const int sizeEntityGroupLstRow = sizeof(typeEntityGroupLstRow) / sizeof(const type_info *);

    // Row adapter to select entity_group_lst rows
    class EntityGroupLstRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new EntityGroupLstRow(); }
        int size(void) const { return sizeEntityGroupLstRow; }
        const type_info * const * columnTypes(void) const { return typeEntityGroupLstRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<EntityGroupLstRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<EntityGroupLstRow *>(i_row)->entityId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<EntityGroupLstRow *>(i_row)->groupId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<EntityGroupLstRow *>(i_row)->name = ((const char *)i_value);
                break;
            case 4:
                dynamic_cast<EntityGroupLstRow *>(i_row)->isHidden = (*(bool *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IEntityGroupLstTable::~IEntityGroupLstTable(void) noexcept { }

// Create new table rows by loading db rows
IEntityGroupLstTable * IEntityGroupLstTable::create(IDbExec * i_dbExec, int i_modelId)
{
    return new EntityGroupLstTable(i_dbExec, i_modelId);
}

// Create new table rows by swap with supplied vector of rows
IEntityGroupLstTable * IEntityGroupLstTable::create(IRowBaseVec & io_rowVec)
{
    return new EntityGroupLstTable(io_rowVec);
}

// Load table
EntityGroupLstTable::EntityGroupLstTable(IDbExec * i_dbExec, int i_modelId)
{ 
    const IRowAdapter & adp = EntityGroupLstRowAdapter();
    rowVec = load(
        "SELECT" \
        " model_id, model_entity_id, group_id, group_name, is_hidden" \
        " FROM entity_group_lst" +
        ((i_modelId > 0) ? " WHERE model_id = " + to_string(i_modelId) : "") +
        " ORDER BY 1, 2, 3", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
EntityGroupLstTable::~EntityGroupLstTable(void) noexcept { }

// Find row by primary key: model id, entity id, group id
const EntityGroupLstRow * EntityGroupLstTable::byKey(int i_modelId, int i_entityId, int i_groupId) const
{
    const IRowBaseUptr keyRow( new EntityGroupLstRow(i_modelId, i_entityId, i_groupId) );
    return findKey(keyRow);
}

// get list of rows by model id
vector<EntityGroupLstRow> EntityGroupLstTable::byModelId(int i_modelId) const
{
    return findAll(
        [i_modelId](const EntityGroupLstRow & i_row) -> bool {
                return i_row.modelId == i_modelId;
            }
        );
}
