// OpenM++ data library: type_dic table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // type_dic table implementation
    class TypeDicTable : public ITypeDicTable
    {
    public:
        TypeDicTable(IDbExec * i_dbExec, int i_modelId = 0);
        TypeDicTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~TypeDicTable() throw();

        // get const reference to list of all table rows
        const IRowBaseVec & rowsCRef(void) const { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by primary key: model id and type id
        const TypeDicRow * byKey(int i_modelId, int i_typeId) const;

        // get list of rows by model id
        vector<TypeDicRow> byModelId(int i_modelId) const;

        // find first row by model id and type name or NULL if not found
        const TypeDicRow * byModelIdName(int i_modelId, const string & i_name) const;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        TypeDicTable(const TypeDicTable & i_table) = delete;
        TypeDicTable & operator=(const TypeDicTable & i_table) = delete;
    };

    // Columns type for type_dic row
    static const type_info * typeTypeDicRow[] = { 
        &typeid(decltype(TypeDicRow::modelId)), 
        &typeid(decltype(TypeDicRow::typeId)), 
        &typeid(decltype(TypeDicRow::name)), 
        &typeid(decltype(TypeDicRow::dicId)), 
        &typeid(decltype(TypeDicRow::totalEnumId))
    };

    // Size (number of columns) for type_dic row
    static const int sizeTypeDicRow = sizeof(typeTypeDicRow) / sizeof(const type_info *);

    // Row adapter to select type_dic rows
    class TypeDicRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new TypeDicRow(); }
        int size(void) const { return sizeTypeDicRow; }
        const type_info ** columnTypes(void) const { return typeTypeDicRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<TypeDicRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<TypeDicRow *>(i_row)->typeId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<TypeDicRow *>(i_row)->name = ((const char *)i_value);
                break;
            case 3:
                dynamic_cast<TypeDicRow *>(i_row)->dicId = (*(int *)i_value);
                break;
            case 4:
                dynamic_cast<TypeDicRow *>(i_row)->totalEnumId = (*(int *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITypeDicTable::~ITypeDicTable(void) throw() { }

// Create new table rows by loading db rows
ITypeDicTable * ITypeDicTable::create(IDbExec * i_dbExec, int i_modelId)
{
    return new TypeDicTable(i_dbExec, i_modelId);
}

// Create new table rows by swap with supplied vector of rows
ITypeDicTable * ITypeDicTable::create(IRowBaseVec & io_rowVec)
{
    return new TypeDicTable(io_rowVec);
}

// Load table
TypeDicTable::TypeDicTable(IDbExec * i_dbExec, int i_modelId)
{ 
    const IRowAdapter & adp = TypeDicRowAdapter();
    rowVec = load(
        "SELECT" \
        " model_id, mod_type_id, mod_type_name, dic_id, total_enum_id" \
        " FROM type_dic" +
        ((i_modelId > 0) ? " WHERE model_id = " + to_string(i_modelId) : "") +
        " ORDER BY 1, 2", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
TypeDicTable::~TypeDicTable(void) throw() { }

// Find row by primary key: model id and type id
const TypeDicRow * TypeDicTable::byKey(int i_modelId, int i_typeId) const
{
    const IRowBaseUptr keyRow( new TypeDicRow(i_modelId, i_typeId) );
    return findKey(keyRow);
}

// get list of rows by model id
vector<TypeDicRow> TypeDicTable::byModelId(int i_modelId) const
{
    return findAll(
        [i_modelId](const TypeDicRow & i_row) -> bool { return i_row.modelId == i_modelId; }
    );
}

// find first row by model id and type name or NULL if not found
const TypeDicRow * TypeDicTable::byModelIdName(int i_modelId, const string & i_name) const
{
    return findFirst(
        [i_modelId, i_name](const TypeDicRow & i_row) -> bool {
                return i_row.modelId == i_modelId && i_row.name == i_name;
            }
        );
}
