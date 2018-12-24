// OpenM++ data library: model_word table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // model_word table implementation
    class ModelWordTable : public IModelWordTable
    {
    public:
        ModelWordTable(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);
        ModelWordTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~ModelWordTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec & rowsCRef(void) const { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by model id, language, word code
        const ModelWordRow * byKey(int i_modelId, int i_langId, const string & i_code) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private: 
        ModelWordTable(const ModelWordTable & i_table) = delete;
        ModelWordTable & operator=(const ModelWordTable & i_table) = delete;
    };

    // Columns type for model_word row
    static const type_info * typeModelWordRow[] = { 
        &typeid(decltype(ModelWordRow::modelId)), 
        &typeid(decltype(ModelWordRow::langId)),
        &typeid(decltype(ModelWordRow::code)), 
        &typeid(decltype(ModelWordRow::value))
    };

    // Size (number of columns) for model_word row
    static const int sizeModelWordRow = sizeof(typeModelWordRow) / sizeof(const type_info *);

    // Row adapter to select model_word rows
    class ModelWordRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new ModelWordRow(); }
        int size(void) const { return sizeModelWordRow; }
        const type_info * const * columnTypes(void) const { return typeModelWordRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<ModelWordRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<ModelWordRow *>(i_row)->langId = *((int *)i_value);
                break;
            case 2:
                dynamic_cast<ModelWordRow *>(i_row)->code = ((const char *)i_value);
                break;
            case 3:
                dynamic_cast<ModelWordRow *>(i_row)->value = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IModelWordTable::~IModelWordTable(void) noexcept { }

// Create new table rows by loading db rows
IModelWordTable * IModelWordTable::create(IDbExec * i_dbExec, int i_modelId, int i_langId)
{
    return new ModelWordTable(i_dbExec, i_modelId, i_langId);
}

// Create new table rows by swap with supplied vector of rows
IModelWordTable * IModelWordTable::create(IRowBaseVec & io_rowVec)
{
    return new ModelWordTable(io_rowVec);
}

// Load table
ModelWordTable::ModelWordTable(IDbExec * i_dbExec, int i_modelId, int i_langId)
{ 
    string sWhere = "";
    if (i_modelId > 0 && i_langId < 0) sWhere = " WHERE model_id = " + to_string(i_modelId);
    if (i_modelId <= 0 && i_langId >= 0) sWhere = " WHERE lang_id = " + to_string(i_langId);
    if (i_modelId > 0 && i_langId >= 0) sWhere = " WHERE model_id = " + to_string(i_modelId) + " AND lang_id = " + to_string(i_langId);

    const IRowAdapter & adp = ModelWordRowAdapter();
    rowVec = load(
        "SELECT model_id, lang_id, word_code, word_value FROM model_word" + sWhere + " ORDER BY 1, 2, 3", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
ModelWordTable::~ModelWordTable(void) noexcept { }

// Find row by primary key: model id, language, word code
const ModelWordRow * ModelWordTable::byKey(int i_modelId, int i_langId, const string & i_code) const
{
    const IRowBaseUptr keyRow( new ModelWordRow(i_modelId, i_langId, i_code) );
    return findKey(keyRow);
}

