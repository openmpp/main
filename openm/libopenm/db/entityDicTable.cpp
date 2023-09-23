// OpenM++ data library: entity_dic join to model_entity_dic table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // entity_dic join to model_entity_dic table implementation
    class EntityDicTable : public IEntityDicTable
    {
    public:
        EntityDicTable(IDbExec * i_dbExec, int i_modelId = 0);
        EntityDicTable(IRowBaseVec & io_rowVec) { rowVec.swap(io_rowVec); }
        ~EntityDicTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec& rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) override { return rowVec; }

        // find row by unique key: model id, model entity id
        const EntityDicRow * byKey(int i_modelId, int i_entityId) const override;

        // get list of rows by model id
        vector<EntityDicRow> byModelId(int i_modelId) const override;

        // find first row by model id and entity name or NULL if not found
        const EntityDicRow * byModelIdName(int i_modelId, const string & i_name) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        EntityDicTable(const EntityDicTable & i_table) = delete;
        EntityDicTable & operator=(const EntityDicTable & i_table) = delete;
    };

    // Columns type for entity_dic join to model_entity_dic row
    static const type_info * typeEntityDicRow[] = { 
        &typeid(decltype(EntityDicRow::modelId)),
        &typeid(decltype(EntityDicRow::entityId)),
        &typeid(decltype(EntityDicRow::entityName)),
        &typeid(decltype(EntityDicRow::entityHid)),
        &typeid(decltype(EntityDicRow::digest))
    };

    // Size (number of columns) for entity_dic join to model_entity_dic row
    static const int sizeEntityDicRow = sizeof(typeEntityDicRow) / sizeof(const type_info *);

    // Row adapter to select entity_dic join to model_entity_dic rows
    class EntityDicRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new EntityDicRow(); }
        int size(void) const { return sizeEntityDicRow; }
        const type_info * const * columnTypes(void) const { return typeEntityDicRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<EntityDicRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<EntityDicRow *>(i_row)->entityId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<EntityDicRow *>(i_row)->entityName = ((const char *)i_value);
                break;
            case 3:
                dynamic_cast<EntityDicRow *>(i_row)->entityHid = (*(int *)i_value);
                break;
            case 4:
                dynamic_cast<EntityDicRow *>(i_row)->digest = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IEntityDicTable::~IEntityDicTable(void) noexcept { }

// Create new table rows by loading db rows
IEntityDicTable * IEntityDicTable::create(IDbExec * i_dbExec, int i_modelId)
{
    return new EntityDicTable(i_dbExec, i_modelId);
}

// Create new table rows by swap with supplied vector of rows
IEntityDicTable * IEntityDicTable::create(IRowBaseVec & io_rowVec)
{
    return new EntityDicTable(io_rowVec);
}

// Load table
EntityDicTable::EntityDicTable(IDbExec * i_dbExec, int i_modelId)   
{ 
    const IRowAdapter & adp = EntityDicRowAdapter();
    rowVec = load(
        "SELECT" \
        " ME.model_id, ME.model_entity_id, D.entity_name, D.entity_hid, D.entity_digest" \
        " FROM entity_dic D" \
        " INNER JOIN model_entity_dic ME ON (ME.entity_hid = D.entity_hid)" +
        ((i_modelId > 0) ? " WHERE ME.model_id = " + to_string(i_modelId) : "") +
        " ORDER BY 1, 2", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
EntityDicTable::~EntityDicTable(void) noexcept { }

// Find row by unique key: model id, model entity id
const EntityDicRow * EntityDicTable::byKey(int i_modelId, int i_entityId) const
{
    const IRowBaseUptr keyRow( new EntityDicRow(i_modelId, i_entityId) );
    return findKey(keyRow);
}

// find first row by model id and entity name or NULL if not found
const EntityDicRow * EntityDicTable::byModelIdName(int i_modelId, const string & i_name) const
{
    return findFirst(
        [i_modelId, i_name](const EntityDicRow & i_row) -> bool {
                return i_row.modelId == i_modelId && i_row.entityName == i_name;
            }
        );
}

// get list of rows by model id
vector<EntityDicRow> EntityDicTable::byModelId(int i_modelId) const
{
    return findAll(
        [i_modelId](const EntityDicRow & i_row) -> bool { return i_row.modelId == i_modelId; }
    );
}
