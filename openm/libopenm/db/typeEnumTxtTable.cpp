// OpenM++ data library: type_enum_txt table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // type_enum_txt table implementation
    class TypeEnumTxtTable : public ITypeEnumTxtTable
    {
    public:
        TypeEnumTxtTable(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);
        TypeEnumTxtTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~TypeEnumTxtTable() throw();

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by primary key: model id, type id, enum id, language id
        const TypeEnumTxtRow * byKey(int i_modelId, int i_typeId, int i_enumId, int i_langId) const;

        // get list of all table rows
        vector<TypeEnumTxtRow> rows(void) const { return IMetaTable<TypeEnumTxtRow>::rows(rowVec); }

        // get list of rows by language
        vector<TypeEnumTxtRow> byLang(int i_langId) const;

        // get list of rows by model id and type id
        vector<TypeEnumTxtRow> byModelIdTypeId(int i_modelId, int i_typeId) const;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        TypeEnumTxtTable(const TypeEnumTxtTable & i_table);               // = delete;
        TypeEnumTxtTable & operator=(const TypeEnumTxtTable & i_table);   // = delete;
    };

    // Columns type for type_enum_txt row
    static const type_info * typeTypeEnumTxtRow[] = { 
        &typeid(decltype(TypeEnumTxtRow::modelId)), 
        &typeid(decltype(TypeEnumTxtRow::typeId)), 
        &typeid(decltype(TypeEnumTxtRow::enumId)), 
        &typeid(decltype(TypeEnumTxtRow::langId)), 
        &typeid(decltype(TypeEnumTxtRow::descr)), 
        &typeid(decltype(TypeEnumTxtRow::note))
    };

    // Size (number of columns) for type_enum_txt row
    static const int sizeTypeEnumTxtRow = sizeof(typeTypeEnumTxtRow) / sizeof(const type_info *);

    // Row adapter to select type_enum_txt rows
    class TypeEnumTxtRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new TypeEnumTxtRow(); }
        int size(void) const { return sizeTypeEnumTxtRow; }
        const type_info ** columnTypes(void) const { return typeTypeEnumTxtRow; }

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
ITypeEnumTxtTable::~ITypeEnumTxtTable(void) throw() { }

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
    if (i_modelId > 0 && i_langId < 0) sWhere = " WHERE model_id = " + to_string(i_modelId);
    if (i_modelId <= 0 && i_langId >= 0) sWhere = " WHERE lang_id = " + to_string(i_langId);
    if (i_modelId > 0 && i_langId >= 0) sWhere = " WHERE model_id = " + to_string(i_modelId) + " AND lang_id = " + to_string(i_langId);

    const IRowAdapter & adp = TypeEnumTxtRowAdapter();
    rowVec = IMetaTable<TypeEnumTxtRow>::load(
        "SELECT model_id, mod_type_id, enum_id, lang_id, descr, note FROM type_enum_txt" + sWhere + " ORDER BY 1, 2, 3, 4", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
TypeEnumTxtTable::~TypeEnumTxtTable(void) throw() { }

// Find row by primary key: model id, type id, language id
const TypeEnumTxtRow * TypeEnumTxtTable::byKey(int i_modelId, int i_typeId, int i_enumId, int i_langId) const
{
    const IRowBaseUptr keyRow( new TypeEnumTxtRow(i_modelId, i_typeId, i_enumId, i_langId) );
    return IMetaTable<TypeEnumTxtRow>::byKey(keyRow, rowVec);
}

// get list of rows by language
vector<TypeEnumTxtRow> TypeEnumTxtTable::byLang(int i_langId) const
{
    const IRowBaseUptr row( new TypeEnumTxtRow(0, 0, 0, i_langId) );
    return IMetaTable<TypeEnumTxtRow>::findAll(row, rowVec, TypeEnumTxtRow::langEqual);
}

// get list of rows by model id and type id
vector<TypeEnumTxtRow> TypeEnumTxtTable::byModelIdTypeId(int i_modelId, int i_typeId) const
{
    const IRowBaseUptr row( new TypeEnumTxtRow(i_modelId, i_typeId, 0, 0) );
    return IMetaTable<TypeEnumTxtRow>::findAll(row, rowVec, TypeEnumTxtRow::modelIdTypeIdEqual);
}
