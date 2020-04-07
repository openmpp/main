// OpenM++ data library: model_dic table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // model_dic table implementation
    class ModelDicTable : public IModelDicTable
    {
    public:
        ModelDicTable(IDbExec * i_dbExec, int i_modelId);
        ModelDicTable(IDbExec * i_dbExec, const char * i_name = nullptr, const char * i_digest = nullptr);
        ModelDicTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~ModelDicTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec & rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) override { return rowVec; }

        // find row by model id
        const ModelDicRow * byKey(int i_modelId) const override;

        // find first row by model name and digest or NULL if not found
        const ModelDicRow * byNameDigest(const string & i_name, const string & i_digest) const override;

    private:
        IRowBaseVec rowVec;     // table rows

        // load table rows from db
        void select(IDbExec * i_dbExec, const string & i_where);

    private:
        ModelDicTable(const ModelDicTable & i_table) = delete;
        ModelDicTable & operator=(const ModelDicTable & i_table) = delete;
    };

    // Columns type for model_dic row
    static const type_info * typeModelDicRow[] = { 
        &typeid(decltype(ModelDicRow::modelId)), 
        &typeid(decltype(ModelDicRow::name)), 
        &typeid(decltype(ModelDicRow::digest)), 
        &typeid(decltype(ModelDicRow::type)),
        &typeid(decltype(ModelDicRow::version)),
        &typeid(decltype(ModelDicRow::createDateTime)),
        &typeid(decltype(ModelDicRow::defaultLangId))
   };

    // Size (number of columns) for model_dic row
    static const int sizeModelDicRow = sizeof(typeModelDicRow) / sizeof(const type_info *);

    // Row adapter to select model_dic rows
    class ModelDicRowAdapter : public IRowAdapter
    {
    public:
        ModelDicRowAdapter() { }

        IRowBase * createRow(void) const { return new ModelDicRow(); }
        int size(void) const { return sizeModelDicRow; }
        const type_info * const * columnTypes(void) const { return typeModelDicRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<ModelDicRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<ModelDicRow *>(i_row)->name = ((const char *)i_value);
                break;
            case 2:
                dynamic_cast<ModelDicRow *>(i_row)->digest = ((const char *)i_value);
                break;
            case 3:
                dynamic_cast<ModelDicRow *>(i_row)->type = (*((int *)i_value));
                break;
            case 4:
                dynamic_cast<ModelDicRow *>(i_row)->version = ((const char *)i_value);
                break;
            case 5:
                dynamic_cast<ModelDicRow *>(i_row)->createDateTime = ((const char *)i_value);
                break;
            case 6:
                dynamic_cast<ModelDicRow *>(i_row)->defaultLangId = (*((int *)i_value));
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IModelDicTable::~IModelDicTable(void) noexcept { }

// Create new table rows by loading db rows by model name and/or digest
IModelDicTable * IModelDicTable::create(IDbExec * i_dbExec, const char * i_name, const char * i_digest)
{
    return new ModelDicTable(i_dbExec, i_name, i_digest);
}

// Create new table rows by loading db rows by primery key: model id
IModelDicTable * IModelDicTable::create(IDbExec * i_dbExec, int i_modelId)
{
    return new ModelDicTable(i_dbExec, i_modelId);
}

// Create new table rows by swap with supplied vector of rows
IModelDicTable * IModelDicTable::create(IRowBaseVec & io_rowVec)
{
    return new ModelDicTable(io_rowVec);
}

// Table never unloaded
ModelDicTable::~ModelDicTable(void) noexcept { }

// Load table rows from db by model name and/or digest
ModelDicTable::ModelDicTable(IDbExec * i_dbExec, const char * i_name, const char * i_digest)
{ 
    string where;
    if (i_name != nullptr && i_digest == nullptr) where += " WHERE model_name = " + toQuoted(i_name);
    if (i_name == nullptr && i_digest != nullptr) where += " WHERE model_digest = " + toQuoted(i_digest);
    if (i_name != nullptr && i_digest != nullptr) where += " WHERE model_name = " + toQuoted(i_name) + " AND model_digest = " + toQuoted(i_digest);

    select(i_dbExec, where);
}

// Load table row from db by primary key: model id
ModelDicTable::ModelDicTable(IDbExec * i_dbExec, int i_modelId)
{
    string where = " WHERE model_id = " + to_string(i_modelId);
    select(i_dbExec, where);
}

// Load table rows from db
void ModelDicTable::select(IDbExec * i_dbExec, const string & i_where)
{
    string sql =
        "SELECT" \
        " model_id, model_name, model_digest, model_type, model_ver, create_dt, default_lang_id" \
        " FROM model_dic" + 
        (!i_where.empty() ? i_where :"") +
        " ORDER BY 1";

    const IRowAdapter & adp = ModelDicRowAdapter();
    rowVec = load(sql, i_dbExec, adp);
}

// Find row by primary key: model id
const ModelDicRow * ModelDicTable::byKey(int i_modelId) const
{
    const IRowBaseUptr keyRow( new ModelDicRow(i_modelId) );
    return findKey(keyRow);
}

// find first row by model name and digest or NULL if not found
const ModelDicRow * ModelDicTable::byNameDigest(const string & i_name, const string & i_digest) const
{
    return findFirst(
        [i_name, i_digest](const ModelDicRow & i_row) -> bool {
            return i_row.name == i_name && i_row.digest == i_digest;
        }
    );
}
