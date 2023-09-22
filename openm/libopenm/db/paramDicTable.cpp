// OpenM++ data library: parameter_dic join to model_parameter_dic table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // parameter_dic join to model_parameter_dic table implementation
    class ParamDicTable : public IParamDicTable
    {
    public:
        ParamDicTable(IDbExec * i_dbExec, int i_modelId = 0);
        ParamDicTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~ParamDicTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec& rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) override { return rowVec; }

        // find row by unique key: model id and model parameter id
        const ParamDicRow * byKey(int i_modelId, int i_paramId) const override;

        // get list of rows by model id
        vector<ParamDicRow> byModelId(int i_modelId) const override;

        // find first row by model id and parameter name or NULL if not found
        const ParamDicRow * byModelIdName(int i_modelId, const string & i_name) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        ParamDicTable(const ParamDicTable & i_table) = delete;
        ParamDicTable & operator=(const ParamDicTable & i_table) = delete;
    };

    // Columns type for parameter_dic join to model_parameter_dic row
    static const type_info * typeParamDicRow[] = { 
        &typeid(decltype(ParamDicRow::modelId)),
        &typeid(decltype(ParamDicRow::paramId)),
        &typeid(decltype(ParamDicRow::paramName)),
        &typeid(decltype(ParamDicRow::paramHid)),
        &typeid(decltype(ParamDicRow::digest)),
        &typeid(decltype(ParamDicRow::rank)),
        &typeid(decltype(ParamDicRow::typeId)),
        &typeid(decltype(ParamDicRow::isExtendable)),
        &typeid(decltype(ParamDicRow::isHidden)),
        &typeid(decltype(ParamDicRow::numCumulated)),
        &typeid(decltype(ParamDicRow::dbRunTable)),
        &typeid(decltype(ParamDicRow::dbSetTable)),
        &typeid(decltype(ParamDicRow::importDigest))
    };

    // Size (number of columns) for parameter_dic join to model_parameter_dic row
    static const int sizeParamDicRow = sizeof(typeParamDicRow) / sizeof(const type_info *);

    // Row adapter to select parameter_dic rows
    class ParamDicRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new ParamDicRow(); }
        int size(void) const { return sizeParamDicRow; }
        const type_info * const * columnTypes(void) const { return typeParamDicRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<ParamDicRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<ParamDicRow *>(i_row)->paramId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<ParamDicRow *>(i_row)->paramName = ((const char *)i_value);
                break;
            case 3:
                dynamic_cast<ParamDicRow *>(i_row)->paramHid = (*(int *)i_value);
                break;
            case 4:
                dynamic_cast<ParamDicRow *>(i_row)->digest = ((const char *)i_value);
                break;
            case 5:
                dynamic_cast<ParamDicRow *>(i_row)->rank = (*(int *)i_value);
                break;
            case 6:
                dynamic_cast<ParamDicRow *>(i_row)->typeId = (*(int *)i_value);
                break;
            case 7:
                dynamic_cast<ParamDicRow *>(i_row)->isExtendable = (*(bool *)i_value);
                break;
            case 8:
                dynamic_cast<ParamDicRow *>(i_row)->isHidden = (*(bool *)i_value);
                break;
            case 9:
                dynamic_cast<ParamDicRow *>(i_row)->numCumulated = (*(int *)i_value);
                break;
            case 10:
                dynamic_cast<ParamDicRow *>(i_row)->dbRunTable = ((const char *)i_value);
                break;
            case 11:
                dynamic_cast<ParamDicRow *>(i_row)->dbSetTable = ((const char *)i_value);
                break;
            case 12:
                dynamic_cast<ParamDicRow *>(i_row)->importDigest = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IParamDicTable::~IParamDicTable(void) noexcept { }

// Create new table rows by loading db rows
IParamDicTable * IParamDicTable::create(IDbExec * i_dbExec, int i_modelId)
{
    return new ParamDicTable(i_dbExec, i_modelId);
}

// Create new table rows by swap with supplied vector of rows
IParamDicTable * IParamDicTable::create(IRowBaseVec & io_rowVec)
{
    return new ParamDicTable(io_rowVec);
}

// Load table
ParamDicTable::ParamDicTable(IDbExec * i_dbExec, int i_modelId)
{ 
    const IRowAdapter & adp = ParamDicRowAdapter();
    rowVec = load(
        "SELECT" \
        " M.model_id, M.model_parameter_id, D.parameter_name, D.parameter_hid," \
        " D.parameter_digest, D.parameter_rank, T.model_type_id, D.is_extendable," \
        " M.is_hidden, D.num_cumulated, D.db_run_table, D.db_set_table, D.import_digest" \
        " FROM parameter_dic D" \
        " INNER JOIN model_parameter_dic M ON (M.parameter_hid = D.parameter_hid)" \
        " INNER JOIN model_type_dic T ON (T.model_id = M.model_id AND T.type_hid = D.type_hid)" +
        ((i_modelId > 0) ? " WHERE M.model_id = " + to_string(i_modelId) : "") +
        " ORDER BY 1, 2", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
ParamDicTable::~ParamDicTable(void) noexcept { }

// Find row by unique key: model id and model parameter id
const ParamDicRow * ParamDicTable::byKey(int i_modelId, int i_paramId) const
{
    const IRowBaseUptr keyRow( new ParamDicRow(i_modelId, i_paramId) );
    return findKey(keyRow);
}

// find first row by model id and parameter name or NULL if not found
const ParamDicRow * ParamDicTable::byModelIdName(int i_modelId, const string & i_name) const
{
    return findFirst(
        [i_modelId, i_name](const ParamDicRow & i_row) -> bool {
                return i_row.modelId == i_modelId && i_row.paramName == i_name;
            }
        );
}

// get list of rows by model id
vector<ParamDicRow> ParamDicTable::byModelId(int i_modelId) const
{
    return findAll(
        [i_modelId](const ParamDicRow & i_row) -> bool { return i_row.modelId == i_modelId; }
    );
}
