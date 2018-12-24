// OpenM++ data library: type_enum_txt join to model_type_dic table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // type_enum_txt join to model_type_dic table implementation
    class TypeEnumTxtTable : public ITypeEnumTxtTable
    {
    public:
        TypeEnumTxtTable(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);
        TypeEnumTxtTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~TypeEnumTxtTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec & rowsCRef(void) const { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by unique key: model id, model type id, enum id, language id
        const TypeEnumTxtRow * byKey(int i_modelId, int i_typeId, int i_enumId, int i_langId) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        TypeEnumTxtTable(const TypeEnumTxtTable & i_table) = delete;
        TypeEnumTxtTable & operator=(const TypeEnumTxtTable & i_table) = delete;
    };

    // Columns type for type_enum_txt join to model_type_dic row
    static const type_info * typeTypeEnumTxtRow[] = { 
        &typeid(decltype(TypeDicTxtRow::modelId)), 
        &typeid(decltype(TypeDicTxtRow::typeId)), 
        &typeid(decltype(TypeEnumTxtRow::enumId)), 
        &typeid(decltype(TypeEnumTxtRow::langId)), 
        &typeid(decltype(TypeEnumTxtRow::descr)), 
        &typeid(decltype(TypeEnumTxtRow::note))
    };

    // Size (number of columns) for type_enum_txt join to model_type_dic row
    static const int sizeTypeEnumTxtRow = sizeof(typeTypeEnumTxtRow) / sizeof(const type_info *);

    // Row adapter to select type_enum_txt join to model_type_dic rows
    class TypeEnumTxtRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new TypeEnumTxtRow(); }
        int size(void) const { return sizeTypeEnumTxtRow; }
        const type_info * const * columnTypes(void) const { return typeTypeEnumTxtRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<TypeEnumTxtRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<TypeEnumTxtRow *>(i_row)->typeId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<TypeEnumTxtRow *>(i_row)->enumId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<TypeEnumTxtRow *>(i_row)->langId = (*(int *)i_value);
                break;
            case 4:
                dynamic_cast<TypeEnumTxtRow *>(i_row)->descr = ((const char *)i_value);
                break;
            case 5:
                dynamic_cast<TypeEnumTxtRow *>(i_row)->note = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITypeEnumTxtTable::~ITypeEnumTxtTable(void) noexcept { }

// Create new table rows by loading db rows
ITypeEnumTxtTable * ITypeEnumTxtTable::create(IDbExec * i_dbExec, int i_modelId, int i_langId)
{
    return new TypeEnumTxtTable(i_dbExec, i_modelId, i_langId);
}

// Create new table rows by swap with supplied vector of rows
ITypeEnumTxtTable * ITypeEnumTxtTable::create(IRowBaseVec & io_rowVec)
{
    return new TypeEnumTxtTable(io_rowVec);
}

// Load table
TypeEnumTxtTable::TypeEnumTxtTable(IDbExec * i_dbExec, int i_modelId, int i_langId)   
{ 
    string sWhere = "";
    if (i_modelId > 0 && i_langId < 0) sWhere = " WHERE M.model_id = " + to_string(i_modelId);
    if (i_modelId <= 0 && i_langId >= 0) sWhere = " WHERE D.lang_id = " + to_string(i_langId);
    if (i_modelId > 0 && i_langId >= 0) sWhere = " WHERE M.model_id = " + to_string(i_modelId) + " AND D.lang_id = " + to_string(i_langId);

    const IRowAdapter & adp = TypeEnumTxtRowAdapter();
    rowVec = load(
        "SELECT  M.model_id, M.model_type_id, D.enum_id, D.lang_id, D.descr, D.note" \
        " FROM type_enum_txt D" \
        " INNER JOIN model_type_dic M ON (M.type_hid = D.type_hid)" +
        sWhere + " ORDER BY 1, 2, 3, 4", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
TypeEnumTxtTable::~TypeEnumTxtTable(void) noexcept { }

// Find row by unique key: model id, model type id, language id
const TypeEnumTxtRow * TypeEnumTxtTable::byKey(int i_modelId, int i_typeId, int i_enumId, int i_langId) const
{
    const IRowBaseUptr keyRow( new TypeEnumTxtRow(i_modelId, i_typeId, i_enumId, i_langId) );
    return findKey(keyRow);
}
