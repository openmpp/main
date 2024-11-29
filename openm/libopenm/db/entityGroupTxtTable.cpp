// OpenM++ data library: entity_group_txt table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // entity_group_txt table implementation
    class EntityGroupTxtTable : public IEntityGroupTxtTable
    {
    public:
        EntityGroupTxtTable(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);
        EntityGroupTxtTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~EntityGroupTxtTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec & rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) override { return rowVec; }

        // find row by primary key: model id, enity id, group id, language id
        const EntityGroupTxtRow * byKey(int i_modelId, int i_entityId, int i_groupId, int i_langId) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        EntityGroupTxtTable(const EntityGroupTxtTable & i_table) = delete;
        EntityGroupTxtTable & operator=(const EntityGroupTxtTable & i_table) = delete;
    };

    // Columns type for entity_group_txt row
    static const type_info * typeEntityGroupTxtRow[] = { 
        &typeid(decltype(EntityGroupTxtRow::modelId)), 
        &typeid(decltype(EntityGroupTxtRow::entityId)), 
        &typeid(decltype(EntityGroupTxtRow::groupId)), 
        &typeid(decltype(EntityGroupTxtRow::langId)), 
        &typeid(decltype(EntityGroupTxtRow::descr)), 
        &typeid(decltype(EntityGroupTxtRow::note))
    };

    // Size (number of columns) for entity_group_txt row
    static const int sizeEntityGroupTxtRow = sizeof(typeEntityGroupTxtRow) / sizeof(const type_info *);

    // Row adapter to select entity_group_txt rows
    class EntityGroupTxtRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new EntityGroupTxtRow(); }
        int size(void) const { return sizeEntityGroupTxtRow; }
        const type_info * const * columnTypes(void) const { return typeEntityGroupTxtRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<EntityGroupTxtRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<EntityGroupTxtRow *>(i_row)->entityId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<EntityGroupTxtRow *>(i_row)->groupId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<EntityGroupTxtRow *>(i_row)->langId = (*(int *)i_value);
                break;
            case 4:
                dynamic_cast<EntityGroupTxtRow *>(i_row)->descr = ((const char *)i_value);
                break;
            case 5:
                dynamic_cast<EntityGroupTxtRow *>(i_row)->note = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IEntityGroupTxtTable::~IEntityGroupTxtTable(void) noexcept { }

// Create new table rows by loading db rows
IEntityGroupTxtTable * IEntityGroupTxtTable::create(IDbExec * i_dbExec, int i_modelId, int i_langId)
{
    return new EntityGroupTxtTable(i_dbExec, i_modelId, i_langId);
}

// Create new table rows by swap with supplied vector of rows
IEntityGroupTxtTable * IEntityGroupTxtTable::create(IRowBaseVec & io_rowVec)
{
    return new EntityGroupTxtTable(io_rowVec);
}

// Load table
EntityGroupTxtTable::EntityGroupTxtTable(IDbExec * i_dbExec, int i_modelId, int i_langId)   
{ 
    string sWhere = "";
    if (i_modelId > 0 && i_langId < 0) sWhere = " WHERE model_id = " + to_string(i_modelId);
    if (i_modelId <= 0 && i_langId >= 0) sWhere = " WHERE lang_id = " + to_string(i_langId);
    if (i_modelId > 0 && i_langId >= 0) sWhere = " WHERE model_id = " + to_string(i_modelId) + " AND lang_id = " + to_string(i_langId);

    const IRowAdapter & adp = EntityGroupTxtRowAdapter();
    rowVec = load(
        "SELECT model_id, model_entity_id, group_id, lang_id, descr, note FROM entity_group_txt" + sWhere + " ORDER BY 1, 2, 3, 4", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
EntityGroupTxtTable::~EntityGroupTxtTable(void) noexcept { }

// Find row by primary key: model id, entity id, group id, language id
const EntityGroupTxtRow * EntityGroupTxtTable::byKey(int i_modelId, int i_entityId, int i_groupId, int i_langId) const
{
    const IRowBaseUptr keyRow( new EntityGroupTxtRow(i_modelId, i_entityId, i_groupId, i_langId) );
    return findKey(keyRow);
}

