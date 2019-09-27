// OpenM++ data library: model_dic_txt table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // model_dic_txt table implementation
    class ModelDicTxtTable : public IModelDicTxtTable
    {
    public:
        ModelDicTxtTable(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);
        ModelDicTxtTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~ModelDicTxtTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec& rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by model id and language
        const ModelDicTxtRow * byKey(int i_modelId, int i_langId) const;

    private:
        IRowBaseVec rowVec;     // table rows

    private: 
        ModelDicTxtTable(const ModelDicTxtTable & i_table) = delete;
        ModelDicTxtTable & operator=(const ModelDicTxtTable & i_table) = delete;
    };

    // Columns type for model_dic_txt row
    static const type_info * typeModelDicTxtRow[] = { 
        &typeid(decltype(ModelDicTxtRow::modelId)), 
        &typeid(decltype(ModelDicTxtRow::langId)),
        &typeid(decltype(ModelDicTxtRow::descr)), 
        &typeid(decltype(ModelDicTxtRow::note))
    };

    // Size (number of columns) for model_dic_txt row
    static const int sizeModelDicTxtRow = sizeof(typeModelDicTxtRow) / sizeof(const type_info *);

    // Row adapter to select model_dic_txt rows
    class ModelDicTxtRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new ModelDicTxtRow(); }
        int size(void) const { return sizeModelDicTxtRow; }
        const type_info * const * columnTypes(void) const { return typeModelDicTxtRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<ModelDicTxtRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<ModelDicTxtRow *>(i_row)->langId = *((int *)i_value);
                break;
            case 2:
                dynamic_cast<ModelDicTxtRow *>(i_row)->descr = ((const char *)i_value);
                break;
            case 3:
                dynamic_cast<ModelDicTxtRow *>(i_row)->note = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IModelDicTxtTable::~IModelDicTxtTable(void) noexcept { }

// Create new table rows by loading db rows
IModelDicTxtTable * IModelDicTxtTable::create(IDbExec * i_dbExec, int i_modelId, int i_langId)
{
    return new ModelDicTxtTable(i_dbExec, i_modelId, i_langId);
}

// Create new table rows by swap with supplied vector of rows
IModelDicTxtTable * IModelDicTxtTable::create(IRowBaseVec & io_rowVec)
{
    return new ModelDicTxtTable(io_rowVec);
}

// Load table
ModelDicTxtTable::ModelDicTxtTable(IDbExec * i_dbExec, int i_modelId, int i_langId)
{ 
    string sWhere = "";
    if (i_modelId > 0 && i_langId < 0) sWhere = " WHERE model_id = " + to_string(i_modelId);
    if (i_modelId <= 0 && i_langId >= 0) sWhere = " WHERE lang_id = " + to_string(i_langId);
    if (i_modelId > 0 && i_langId >= 0) sWhere = " WHERE model_id = " + to_string(i_modelId) + " AND lang_id = " + to_string(i_langId);

    const IRowAdapter & adp = ModelDicTxtRowAdapter();
    rowVec = load(
        "SELECT model_id, lang_id, descr, note FROM model_dic_txt" + sWhere + " ORDER BY 1, 2", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
ModelDicTxtTable::~ModelDicTxtTable(void) noexcept { }

// Find row by primary key: model id and language
const ModelDicTxtRow * ModelDicTxtTable::byKey(int i_modelId, int i_langId) const
{
    const IRowBaseUptr keyRow( new ModelDicTxtRow(i_modelId, i_langId) );
    return findKey(keyRow);
}

