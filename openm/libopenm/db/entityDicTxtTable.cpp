// OpenM++ data library: entity_dic_txt join to model_entity_dic table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // entity_dic_txt join to model_entity_dic table implementation
    class EntityDicTxtTable : public IEntityDicTxtTable
    {
    public:
        EntityDicTxtTable(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);
        EntityDicTxtTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~EntityDicTxtTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec& rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) override { return rowVec; }

        // find row by unique key: model id, model entity id, language id
        const EntityDicTxtRow * byKey(int i_modelId, int i_entityId, int i_langId) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        EntityDicTxtTable(const EntityDicTxtTable & i_table) = delete;
        EntityDicTxtTable & operator=(const EntityDicTxtTable & i_table) = delete;
    };

    // Columns type for entity_dic_txt join to model_entity_dic row
    static const type_info * typeEntityDicTxtRow[] = {
        &typeid(decltype(EntityDicTxtRow::modelId)),
        &typeid(decltype(EntityDicTxtRow::entityId)),
        &typeid(decltype(EntityDicTxtRow::langId)),
        &typeid(decltype(EntityDicTxtRow::descr)),
        &typeid(decltype(EntityDicTxtRow::note))
    };

    // Size (number of columns) for entity_dic_txt join to model_entity_dic row
    static const int sizeEntityDicTxtRow = sizeof(typeEntityDicTxtRow) / sizeof(const type_info *);

    // Row adapter to select entity_dic_txt join to model_entity_dic rows
    class EntityDicTxtRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new EntityDicTxtRow(); }
        int size(void) const { return sizeEntityDicTxtRow; }
        const type_info * const * columnTypes(void) const { return typeEntityDicTxtRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<EntityDicTxtRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<EntityDicTxtRow *>(i_row)->entityId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<EntityDicTxtRow *>(i_row)->langId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<EntityDicTxtRow *>(i_row)->descr = ((const char *)i_value);
                break;
            case 4:
                dynamic_cast<EntityDicTxtRow *>(i_row)->note = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IEntityDicTxtTable::~IEntityDicTxtTable(void) noexcept { }

// Create new table rows by loading db rows
IEntityDicTxtTable * IEntityDicTxtTable::create(IDbExec * i_dbExec, int i_modelId, int i_langId)
{
    return new EntityDicTxtTable(i_dbExec, i_modelId, i_langId);
}

// Create new table rows by swap with supplied vector of rows
IEntityDicTxtTable * IEntityDicTxtTable::create(IRowBaseVec & io_rowVec)
{
    return new EntityDicTxtTable(io_rowVec);
}

// Load table
EntityDicTxtTable::EntityDicTxtTable(IDbExec * i_dbExec, int i_modelId, int i_langId)
{ 
    string sWhere = "";
    if (i_modelId > 0 && i_langId < 0) sWhere = " WHERE ME.model_id = " + to_string(i_modelId);
    if (i_modelId <= 0 && i_langId >= 0) sWhere = " WHERE T.lang_id = " + to_string(i_langId);
    if (i_modelId > 0 && i_langId >= 0) sWhere = " WHERE ME.model_id = " + to_string(i_modelId) + " AND T.lang_id = " + to_string(i_langId);

    const IRowAdapter & adp = EntityDicTxtRowAdapter();
    rowVec = load(
        "SELECT ME.model_id, ME.model_entity_id, T.lang_id, T.descr, T.note" \
        " FROM entity_dic_txt T" \
        " INNER JOIN model_entity_dic ME ON (ME.entity_hid = T.entity_hid)" +
        sWhere + 
        " ORDER BY 1, 2, 3", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
EntityDicTxtTable::~EntityDicTxtTable(void) noexcept { }

// Find row by unique key: model id, model entity id, language id
const EntityDicTxtRow * EntityDicTxtTable::byKey(int i_modelId, int i_entityId, int i_langId) const
{
    const IRowBaseUptr keyRow( new EntityDicTxtRow(i_modelId, i_entityId, i_langId) );
    return findKey(keyRow);
}

