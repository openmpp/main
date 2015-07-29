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
        ModelDicTable(IDbExec * i_dbExec, const char * i_name = NULL, const char * i_timestamp = NULL);
        ModelDicTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~ModelDicTable() throw();

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by model id
        const ModelDicRow * byKey(int i_modelId) const;

        // return first table row or NULL if table is empty
        const ModelDicRow * firstRow(void) const { return IMetaTable<ModelDicRow>::firstRow(rowVec); }

        // get list of all table rows
        vector<ModelDicRow> rows(void) const { return IMetaTable<ModelDicRow>::rows(rowVec); }

        // find first row by model name and timestamp or NULL if not found
        const ModelDicRow * byNameTimeStamp(const string & i_name, const string & i_timestamp) const;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        ModelDicTable(const ModelDicTable & i_table);               // = delete;
        ModelDicTable & operator=(const ModelDicTable & i_table);   // = delete;
    };

    // Columns type for model_dic row
    static const type_info * typeModelDicRow[] = { 
        &typeid(decltype(ModelDicRow::modelId)), 
        &typeid(decltype(ModelDicRow::name)), 
        &typeid(decltype(ModelDicRow::type)),
        &typeid(decltype(ModelDicRow::version)),
        &typeid(decltype(ModelDicRow::timestamp)),
        &typeid(decltype(ModelDicRow::modelPrefix)),
        &typeid(decltype(ModelDicRow::paramPrefix)),
        &typeid(decltype(ModelDicRow::setPrefix)),
        &typeid(decltype(ModelDicRow::accPrefix)),
        &typeid(decltype(ModelDicRow::valuePrefix))
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
        const type_info ** columnTypes(void) const { return typeModelDicRow; }

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
                dynamic_cast<ModelDicRow *>(i_row)->type = (*((int *)i_value));
                break;
            case 3:
                dynamic_cast<ModelDicRow *>(i_row)->version = ((const char *)i_value);
                break;
            case 4:
                dynamic_cast<ModelDicRow *>(i_row)->timestamp = ((const char *)i_value);
                break;
            case 5:
                dynamic_cast<ModelDicRow *>(i_row)->modelPrefix = ((const char *)i_value);
                break;
            case 6:
                dynamic_cast<ModelDicRow *>(i_row)->paramPrefix = ((const char *)i_value);
                break;
            case 7:
                dynamic_cast<ModelDicRow *>(i_row)->setPrefix = ((const char *)i_value);
                break;
            case 8:
                dynamic_cast<ModelDicRow *>(i_row)->accPrefix = ((const char *)i_value);
                break;
            case 9:
                dynamic_cast<ModelDicRow *>(i_row)->valuePrefix = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IModelDicTable::~IModelDicTable(void) throw() { }

// Create new table rows by loading db rows
IModelDicTable * IModelDicTable::create(IDbExec * i_dbExec, const char * i_name, const char * i_timestamp)
{
    return new ModelDicTable(i_dbExec, i_name, i_timestamp);
}

// Create new table rows by swap with supplied vector of rows
IModelDicTable * IModelDicTable::create(IRowBaseVec & io_rowVec)
{
    return new ModelDicTable(io_rowVec);
}

// Load table from db
ModelDicTable::ModelDicTable(IDbExec * i_dbExec, const char * i_name, const char * i_timestamp)
{ 
    string sql = 
        "SELECT" \
        " model_id, model_name, model_type, model_ver, model_ts," \
        " model_prefix, parameter_prefix, workset_prefix, acc_prefix, value_prefix" \
        " FROM model_dic";
    if (i_name != NULL && i_timestamp == NULL) sql += " WHERE model_name = " + toQuoted(i_name);
    if (i_name == NULL && i_timestamp != NULL) sql += " WHERE model_ts = " + toQuoted(i_timestamp);
    if (i_name != NULL && i_timestamp != NULL) sql += " WHERE model_name = " + toQuoted(i_name) + " AND model_ts = " + toQuoted(i_timestamp);
    sql += " ORDER BY 1";

    const IRowAdapter & adp = ModelDicRowAdapter();
    rowVec = IMetaTable<ModelDicRow>::load(sql, i_dbExec, adp);
}

// Table never unloaded
ModelDicTable::~ModelDicTable(void) throw() { }

// Find row by primary key: model id
const ModelDicRow * ModelDicTable::byKey(int i_modelId) const
{
    const IRowBaseUptr keyRow( new ModelDicRow(i_modelId) );
    return IMetaTable<ModelDicRow>::byKey(keyRow, rowVec);
}

// find first row by model name and timestamp or NULL if not found
const ModelDicRow * ModelDicTable::byNameTimeStamp(const string & i_name, const string & i_timestamp) const
{
    const IRowBaseUptr row( new ModelDicRow() );
    dynamic_cast<ModelDicRow *>(row.get())->name = i_name;
    dynamic_cast<ModelDicRow *>(row.get())->timestamp = i_timestamp;

    return IMetaTable<ModelDicRow>::findFirst(row, rowVec, ModelDicRow::nameTimeStampEqual);
}
