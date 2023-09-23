// OpenM++ data library: entity_attr_txt join to model_entity_dic table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // entity_attr_txt join to model_entity_dic table implementation
    class EntityAttrTxtTable : public IEntityAttrTxtTable
    {
    public:
        EntityAttrTxtTable(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);
        EntityAttrTxtTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~EntityAttrTxtTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec& rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) override { return rowVec; }

        // find row by unique key: model id, model entity id, attribute id, language id
        const EntityAttrTxtRow * byKey(int i_modelId, int i_entityId, int i_attrId, int i_langId) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        EntityAttrTxtTable(const EntityAttrTxtTable & i_table) = delete;
        EntityAttrTxtTable & operator=(const EntityAttrTxtTable & i_table) = delete;
    };

    // Columns type for entity_attr_txt join to model_entity_dic row
    static const type_info * typeEntityAttrTxtRow[] = { 
        &typeid(decltype(EntityAttrTxtRow::modelId)), 
        &typeid(decltype(EntityAttrTxtRow::entityId)), 
        &typeid(decltype(EntityAttrTxtRow::attrId)),
        &typeid(decltype(EntityAttrTxtRow::langId)), 
        &typeid(decltype(EntityAttrTxtRow::descr)), 
        &typeid(decltype(EntityAttrTxtRow::note)) 
    };

    // Size (number of columns) for entity_attr_txt join to model_entity_dic row
    static const int sizeEntityAttrTxtRow = sizeof(typeEntityAttrTxtRow) / sizeof(const type_info *);

    // Row adapter to select entity_attr_txt join to model_entity_dic rows
    class EntityAttrTxtRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new EntityAttrTxtRow(); }
        int size(void) const { return sizeEntityAttrTxtRow; }
        const type_info * const * columnTypes(void) const { return typeEntityAttrTxtRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<EntityAttrTxtRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<EntityAttrTxtRow *>(i_row)->entityId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<EntityAttrTxtRow *>(i_row)->attrId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<EntityAttrTxtRow *>(i_row)->langId = (*(int *)i_value);
                break;
            case 4:
                dynamic_cast<EntityAttrTxtRow *>(i_row)->descr = ((const char *)i_value);
                break;
            case 5:
                dynamic_cast<EntityAttrTxtRow *>(i_row)->note = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IEntityAttrTxtTable::~IEntityAttrTxtTable(void) noexcept { }

// Create new table rows by loading db rows
IEntityAttrTxtTable * IEntityAttrTxtTable::create(IDbExec * i_dbExec, int i_modelId, int i_langId)
{
    return new EntityAttrTxtTable(i_dbExec, i_modelId, i_langId);
}

// Create new table rows by swap with supplied vector of rows
IEntityAttrTxtTable * IEntityAttrTxtTable::create(IRowBaseVec & io_rowVec)
{
    return new EntityAttrTxtTable(io_rowVec);
}

// Load table
EntityAttrTxtTable::EntityAttrTxtTable(IDbExec * i_dbExec, int i_modelId, int i_langId)   
{ 
    string sWhere = "";
    if (i_modelId > 0 && i_langId < 0) sWhere = " WHERE ME.model_id = " + to_string(i_modelId);
    if (i_modelId <= 0 && i_langId >= 0) sWhere = " WHERE D.lang_id = " + to_string(i_langId);
    if (i_modelId > 0 && i_langId >= 0) sWhere = " WHERE ME.model_id = " + to_string(i_modelId) + " AND D.lang_id = " + to_string(i_langId);

    const IRowAdapter & adp = EntityAttrTxtRowAdapter();
    rowVec = load(
        "SELECT ME.model_id, ME.model_entity_id, D.attr_id, D.lang_id, D.descr, D.note" \
        " FROM entity_attr_txt D" \
        " INNER JOIN model_entity_dic ME ON (ME.entity_hid = D.entity_hid)" +
        sWhere + 
        " ORDER BY 1, 2, 3, 4", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
EntityAttrTxtTable::~EntityAttrTxtTable(void) noexcept { }

// Find row by unique key: model id, model entity id, attribute id, language id
const EntityAttrTxtRow * EntityAttrTxtTable::byKey(int i_modelId, int i_entityId, int i_attrId, int i_langId) const
{
    const IRowBaseUptr keyRow( new EntityAttrTxtRow(i_modelId, i_entityId, i_attrId, i_langId) );
    return findKey(keyRow);
}

