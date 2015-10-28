// OpenM++ data library: type_enum_lst table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // type_enum_lst table implementation
    class TypeEnumLstTable : public ITypeEnumLstTable
    {
    public:
        TypeEnumLstTable(IDbExec * i_dbExec, int i_modelId = 0);
        TypeEnumLstTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~TypeEnumLstTable() throw();

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by primary key: model id, type id, enum id
        const TypeEnumLstRow * byKey(int i_modelId, int i_typeId, int i_enumId) const;

        // get list of all table rows
        vector<TypeEnumLstRow> rows(void) const { return IMetaTable<TypeEnumLstRow>::rows(rowVec); }

        // get list of rows by model id
        vector<TypeEnumLstRow> byModelId(int i_modelId) const;

        // get list of rows by model id and type id
        vector<TypeEnumLstRow> byModelIdTypeId(int i_modelId, int i_typeId) const;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        TypeEnumLstTable(const TypeEnumLstTable & i_table) = delete;
        TypeEnumLstTable & operator=(const TypeEnumLstTable & i_table) = delete;
    };

    // Columns type for type_enum_lst row
    static const type_info * typeTypeEnumLstRow[] = { 
        &typeid(decltype(TypeEnumLstRow::modelId)), 
        &typeid(decltype(TypeEnumLstRow::typeId)), 
        &typeid(decltype(TypeEnumLstRow::enumId)), 
        &typeid(decltype(TypeEnumLstRow::name)) 
    };

    // Size (number of columns) for type_enum_lst row
    static const int sizeTypeEnumLstRow = sizeof(typeTypeEnumLstRow) / sizeof(const type_info *);

    // Row adapter to select type_enum_lst rows
    class TypeEnumLstRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new TypeEnumLstRow(); }
        int size(void) const { return sizeTypeEnumLstRow; }
        const type_info ** columnTypes(void) const { return typeTypeEnumLstRow; }

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
    rowVec = IMetaTable<TypeEnumLstRow>::load(
        "SELECT" \
        " model_id, mod_type_id, enum_id, enum_name" \
        " FROM type_enum_lst" +
        ((i_modelId > 0) ? " WHERE model_id = " + to_string(i_modelId) : "") +
        " ORDER BY 1, 2, 3", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
TypeEnumLstTable::~TypeEnumLstTable(void) throw() { }

// Find row by primary key: model id, type id, enum id
const TypeEnumLstRow * TypeEnumLstTable::byKey(int i_modelId, int i_typeId, int i_enumId) const
{
    const IRowBaseUptr keyRow( new TypeEnumLstRow(i_modelId, i_typeId, i_enumId) );
    return IMetaTable<TypeEnumLstRow>::byKey(keyRow, rowVec);
}

// get list of rows by model id
vector<TypeEnumLstRow> TypeEnumLstTable::byModelId(int i_modelId) const
{
    const IRowBaseUptr row( new TypeEnumLstRow(i_modelId, 0, 0) );
    return IMetaTable<TypeEnumLstRow>::findAll(row, rowVec, TypeEnumLstRow::modelIdEqual);
}

// get list of rows by model id and type id
vector<TypeEnumLstRow> TypeEnumLstTable::byModelIdTypeId(int i_modelId, int i_typeId) const
{
    const IRowBaseUptr row( new TypeEnumLstRow(i_modelId, i_typeId, 0) );
    return IMetaTable<TypeEnumLstRow>::findAll(row, rowVec, TypeEnumLstRow::modelIdTypeIdEqual);
}
