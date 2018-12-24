// OpenM++ data library: type_dic join to model_type_dic table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // type_dic join to model_type_dic table row implementation
    class TypeDicTable : public ITypeDicTable
    {
    public:
        TypeDicTable(IDbExec * i_dbExec, int i_modelId = 0);
        TypeDicTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~TypeDicTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec & rowsCRef(void) const { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by unique key: model id, model type id
        const TypeDicRow * byKey(int i_modelId, int i_typeId) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        TypeDicTable(const TypeDicTable & i_table) = delete;
        TypeDicTable & operator=(const TypeDicTable & i_table) = delete;
    };

    // Columns type for type_dic join to model_type_dic
    static const type_info * typeTypeDicRow[] = { 
        &typeid(decltype(TypeDicRow::modelId)), 
        &typeid(decltype(TypeDicRow::typeId)), 
        &typeid(decltype(TypeDicRow::name)), 
        &typeid(decltype(TypeDicRow::digest)), 
        &typeid(decltype(TypeDicRow::dicId)), 
        &typeid(decltype(TypeDicRow::totalEnumId))
    };

    // Size (number of columns) for type_dic join to model_type_dic
    static const int sizeTypeDicRow = sizeof(typeTypeDicRow) / sizeof(const type_info *);

    // Row adapter to select type_dic rows
    class TypeDicRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new TypeDicRow(); }
        int size(void) const { return sizeTypeDicRow; }
        const type_info * const * columnTypes(void) const { return typeTypeDicRow; }

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
                dynamic_cast<TypeDicRow *>(i_row)->digest = ((const char *)i_value);
                break;
            case 4:
                dynamic_cast<TypeDicRow *>(i_row)->dicId = (*(int *)i_value);
                break;
            case 5:
                dynamic_cast<TypeDicRow *>(i_row)->totalEnumId = (*(int *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITypeDicTable::~ITypeDicTable(void) noexcept { }

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
        " M.model_id, M.model_type_id, H.type_name, H.type_digest, H.dic_id, H.total_enum_id" \
        " FROM type_dic H" \
        " INNER JOIN model_type_dic M ON (M.type_hid = H.type_hid)" +
        ((i_modelId > 0) ? " WHERE M.model_id = " + to_string(i_modelId) : "") +
        " ORDER BY 1, 2", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
TypeDicTable::~TypeDicTable(void) noexcept { }

// find row by unique key: model id, model type id
const TypeDicRow * TypeDicTable::byKey(int i_modelId, int i_typeId) const 
{
    const IRowBaseUptr keyRow( new TypeDicRow(i_modelId, i_typeId) );
    return findKey(keyRow);
}

