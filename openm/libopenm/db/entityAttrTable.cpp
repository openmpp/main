// OpenM++ data library: entity_attr join to model_entity_dic table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // entity_attr join to model_entity_dic table implementation
    class EntityAttrTable : public IEntityAttrTable
    {
    public:
        EntityAttrTable(IDbExec * i_dbExec, int i_modelId = 0);
        EntityAttrTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~EntityAttrTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec& rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) override { return rowVec; }

        // find row by unique key: model id, model entity id, attribute id
        const EntityAttrRow * byKey(int i_modelId, int i_entityId, int i_attrId) const override;

        // get list of rows by model id and entity id
        vector<EntityAttrRow> byModelIdEntityId(int i_modelId, int i_entityId) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        EntityAttrTable(const EntityAttrTable & i_table) = delete;
        EntityAttrTable & operator=(const EntityAttrTable & i_table) = delete;
    };

    // Columns type for entity_attr join to model_entity_dic row
    static const type_info * typeEntityAttrRow[] = { 
        &typeid(decltype(EntityAttrRow::modelId)),
        &typeid(decltype(EntityAttrRow::entityId)),
        &typeid(decltype(EntityAttrRow::attrId)),
        &typeid(decltype(EntityAttrRow::name)),
        &typeid(decltype(EntityAttrRow::typeId)),
        &typeid(decltype(EntityAttrRow::isInternal))
    };

    // Size (number of columns) for entity_attr join to model_entity_dic row
    static const int sizeEntityAttrRow = sizeof(typeEntityAttrRow) / sizeof(const type_info *);

    // Row adapter to select entity_attr join to model_entity_dic rows
    class EntityAttrRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new EntityAttrRow(); }
        int size(void) const { return sizeEntityAttrRow; }
        const type_info * const * columnTypes(void) const { return typeEntityAttrRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<EntityAttrRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<EntityAttrRow *>(i_row)->entityId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<EntityAttrRow *>(i_row)->attrId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<EntityAttrRow *>(i_row)->name = ((const char *)i_value);
                break;
            case 4:
                dynamic_cast<EntityAttrRow *>(i_row)->typeId = (*(int *)i_value);
                break;
            case 5:
                dynamic_cast<EntityAttrRow *>(i_row)->isInternal = (*(bool *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IEntityAttrTable::~IEntityAttrTable(void) noexcept { }

// Create new table rows by loading db rows
IEntityAttrTable * IEntityAttrTable::create(IDbExec * i_dbExec, int i_modelId)
{
    return new EntityAttrTable(i_dbExec, i_modelId);
}

// Create new table rows by swap with supplied vector of rows
IEntityAttrTable * IEntityAttrTable::create(IRowBaseVec & io_rowVec)
{
    return new EntityAttrTable(io_rowVec);
}

// Load table
EntityAttrTable::EntityAttrTable(IDbExec * i_dbExec, int i_modelId)   
{ 
    const IRowAdapter & adp = EntityAttrRowAdapter();
    rowVec = load(
        "SELECT" \
        " M.model_id, M.model_entity_id, D.attr_id, D.attr_name, T.model_type_id, D.is_internal" \
        " FROM entity_attr D" \
        " INNER JOIN model_entity_dic M ON (M.entity_hid = D.entity_hid)" \
        " INNER JOIN model_type_dic T ON (T.type_hid = D.type_hid)" +
        ((i_modelId > 0) ? " WHERE M.model_id = " + to_string(i_modelId) : "") +
        " ORDER BY 1, 2, 3", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
EntityAttrTable::~EntityAttrTable(void) noexcept { }

// Find row by unique key: model id, model entity id, attribute id
const EntityAttrRow * EntityAttrTable::byKey(int i_modelId, int i_entityId, int i_attrId) const
{
    const IRowBaseUptr keyRow( new EntityAttrRow(i_modelId, i_entityId, i_attrId) );
    return findKey(keyRow);
}

// get list of rows by model id and entity id
vector<EntityAttrRow> EntityAttrTable::byModelIdEntityId(int i_modelId, int i_entityId) const
{
    return findAll(
        [i_modelId, i_entityId](const EntityAttrRow & i_row) -> bool {
                return i_row.modelId == i_modelId && i_row.entityId == i_entityId;
            }
    );
}
