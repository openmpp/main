// OpenM++ data library: type_dic_txt join to model_type_dic table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // type_dic_txt join to model_type_dic table implementation
    class TypeDicTxtTable : public ITypeDicTxtTable
    {
    public:
        TypeDicTxtTable(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);
        TypeDicTxtTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~TypeDicTxtTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec& rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by unique key: model id, model type id, language id
        const TypeDicTxtRow * byKey(int i_modelId, int i_typeId, int i_langId) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        TypeDicTxtTable(const TypeDicTxtTable & i_table) = delete;
        TypeDicTxtTable & operator=(const TypeDicTxtTable & i_table) = delete;
    };

    // Columns type for type_dic_txt join to model_type_dic row
    static const type_info * typeTypeDicTxtRow[] = { 
        &typeid(decltype(TypeDicTxtRow::modelId)), 
        &typeid(decltype(TypeDicTxtRow::typeId)), 
        &typeid(decltype(TypeDicTxtRow::langId)), 
        &typeid(decltype(TypeDicTxtRow::descr)), 
        &typeid(decltype(TypeDicTxtRow::note))
    };

    // Size (number of columns) for type_dic_txt join to model_type_dic row
    static const int sizeTypeDicTxtRow = sizeof(typeTypeDicTxtRow) / sizeof(const type_info *);

    // Row adapter to select type_dic_txt join to model_type_dic rows
    class TypeDicTxtRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new TypeDicTxtRow(); }
        int size(void) const { return sizeTypeDicTxtRow; }
        const type_info * const * columnTypes(void) const { return typeTypeDicTxtRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<TypeDicTxtRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<TypeDicTxtRow *>(i_row)->typeId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<TypeDicTxtRow *>(i_row)->langId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<TypeDicTxtRow *>(i_row)->descr = ((const char *)i_value);
                break;
            case 4:
                dynamic_cast<TypeDicTxtRow *>(i_row)->note = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITypeDicTxtTable::~ITypeDicTxtTable(void) noexcept { }

// Create new table rows by loading db rows
ITypeDicTxtTable * ITypeDicTxtTable::create(IDbExec * i_dbExec, int i_modelId, int i_langId)
{
    return new TypeDicTxtTable(i_dbExec, i_modelId, i_langId);
}

// Create new table rows by swap with supplied vector of rows
ITypeDicTxtTable * ITypeDicTxtTable::create(IRowBaseVec & io_rowVec)
{
    return new TypeDicTxtTable(io_rowVec);
}

// Load table
TypeDicTxtTable::TypeDicTxtTable(IDbExec * i_dbExec, int i_modelId, int i_langId)   
{ 
    string sWhere = "";
    if (i_modelId > 0 && i_langId < 0) sWhere = " WHERE M.model_id = " + to_string(i_modelId);
    if (i_modelId <= 0 && i_langId >= 0) sWhere = " WHERE D.lang_id = " + to_string(i_langId);
    if (i_modelId > 0 && i_langId >= 0) sWhere = " WHERE M.model_id = " + to_string(i_modelId) + " AND D.lang_id = " + to_string(i_langId);

    const IRowAdapter & adp = TypeDicTxtRowAdapter();
    rowVec = load(
        "SELECT M.model_id, M.model_type_id, D.lang_id, D.descr, D.note" \
        " FROM type_dic_txt D" \
        " INNER JOIN model_type_dic M ON (M.type_hid = D.type_hid)" +
        sWhere + " ORDER BY 1, 2, 3", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
TypeDicTxtTable::~TypeDicTxtTable(void) noexcept { }

// Find row by unique key: model id, model type id, language id
const TypeDicTxtRow * TypeDicTxtTable::byKey(int i_modelId, int i_typeId, int i_langId) const
{
    const IRowBaseUptr keyRow( new TypeDicTxtRow(i_modelId, i_typeId, i_langId) );
    return findKey(keyRow);
}
