// OpenM++ data library: type_enum_lst join to model_type_dic table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // type_enum_lst join to model_type_dic table implementation
    class TypeEnumLstTable : public ITypeEnumLstTable
    {
    public:
        TypeEnumLstTable(IDbExec * i_dbExec, int i_modelId = 0);
        TypeEnumLstTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~TypeEnumLstTable() throw();

        // get const reference to list of all table rows
        const IRowBaseVec & rowsCRef(void) const { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by unique key: model id, model type id, enum id
        const TypeEnumLstRow * byKey(int i_modelId, int i_typeId, int i_enumId) const override;
        
        // get list of rows by model id
        vector<TypeEnumLstRow> byModelId(int i_modelId) const override;

        // get list of rows by model id and model type id
        vector<TypeEnumLstRow> byModelIdTypeId(int i_modelId, int i_typeId) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        TypeEnumLstTable(const TypeEnumLstTable & i_table) = delete;
        TypeEnumLstTable & operator=(const TypeEnumLstTable & i_table) = delete;
    };

    // Columns type for type_enum_lst join to model_type_dic row
    static const type_info * typeTypeEnumLstRow[] = { 
        &typeid(decltype(TypeDicTxtRow::modelId)), 
        &typeid(decltype(TypeDicTxtRow::typeId)), 
        &typeid(decltype(TypeEnumLstRow::enumId)), 
        &typeid(decltype(TypeEnumLstRow::name)) 
    };

    // Size (number of columns) for type_enum_lst join to model_type_dic row
    static const int sizeTypeEnumLstRow = sizeof(typeTypeEnumLstRow) / sizeof(const type_info *);

    // Row adapter to select type_enum_lst join to model_type_dic rows
    class TypeEnumLstRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new TypeEnumLstRow(); }
        int size(void) const { return sizeTypeEnumLstRow; }
        const type_info * const * columnTypes(void) const { return typeTypeEnumLstRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<TypeEnumLstRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<TypeEnumLstRow *>(i_row)->typeId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<TypeEnumLstRow *>(i_row)->enumId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<TypeEnumLstRow *>(i_row)->name = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITypeEnumLstTable::~ITypeEnumLstTable(void) throw() { }

// Create new table rows by loading db rows
ITypeEnumLstTable * ITypeEnumLstTable::create(IDbExec * i_dbExec, int i_modelId)
{
    return new TypeEnumLstTable(i_dbExec, i_modelId);
}

// Create new table rows by swap with supplied vector of rows
ITypeEnumLstTable * ITypeEnumLstTable::create(IRowBaseVec & io_rowVec)
{
    return new TypeEnumLstTable(io_rowVec);
}

// Load table
TypeEnumLstTable::TypeEnumLstTable(IDbExec * i_dbExec, int i_modelId)
{ 
    const IRowAdapter & adp = TypeEnumLstRowAdapter();
    rowVec = load(
        "SELECT" \
        " M.model_id, M.model_type_id, D.enum_id, D.enum_name" \
        " FROM type_enum_lst D" \
        " INNER JOIN model_type_dic M ON (M.type_hid = D.type_hid)" +
        ((i_modelId > 0) ? " WHERE M.model_id = " + to_string(i_modelId) : "") +
        " ORDER BY 1, 2, 3", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
TypeEnumLstTable::~TypeEnumLstTable(void) throw() { }

// Find row by unique key: model id, model type id, enum id
const TypeEnumLstRow * TypeEnumLstTable::byKey(int i_modelId, int i_typeId, int i_enumId) const
{
    const IRowBaseUptr keyRow( new TypeEnumLstRow(i_modelId, i_typeId, i_enumId) );
    return findKey(keyRow);
}

// get list of rows by model id
vector<TypeEnumLstRow> TypeEnumLstTable::byModelId(int i_modelId) const
{
    return findAll(
        [i_modelId](const TypeEnumLstRow & i_row) -> bool { return i_row.modelId == i_modelId; }
    );
}

// get list of rows by model id and model type id
vector<TypeEnumLstRow> TypeEnumLstTable::byModelIdTypeId(int i_modelId, int i_typeId) const
{
    return findAll(
        [i_modelId, i_typeId](const TypeEnumLstRow & i_row) -> bool {
            return i_row.modelId == i_modelId && i_row.typeId == i_typeId;
        }
    );
}
