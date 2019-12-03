// OpenM++ data library: parameter_dic join to model_parameter_import table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // parameter_dic join to model_parameter_import table implementation
    class ParamImportTable : public IParamImportTable
    {
    public:
        ParamImportTable(IDbExec * i_dbExec, int i_modelId = 0);
        ParamImportTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~ParamImportTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec& rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) override { return rowVec; }

        // find row by unique key: model id, model parameter id, is_from_parameter, from_name, from_model_name
        const ParamImportRow * byKey(int i_modelId, int i_paramId, bool i_isFromParam, const string & i_fromName, const string & i_fromModel) const override;

        // get list of rows by model id
        vector<ParamImportRow> byModelId(int i_modelId) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        ParamImportTable(const ParamImportTable & i_table) = delete;
        ParamImportTable & operator=(const ParamImportTable & i_table) = delete;
    };

    // Columns type for parameter_dic join to model_parameter_import row
    static const type_info * typeParamImportRow[] = { 
        &typeid(decltype(ParamImportRow::modelId)), 
        &typeid(decltype(ParamImportRow::paramId)), 
        &typeid(decltype(ParamImportRow::paramHid)),
        &typeid(decltype(ParamImportRow::isFromParam)),
        &typeid(decltype(ParamImportRow::fromName)),
        & typeid(decltype(ParamImportRow::fromModel)),
        & typeid(decltype(ParamImportRow::isSampleDim))
    };
    // Size (number of columns) for parameter_dic join to model_parameter_import row
    static const int sizeParamImportRow = sizeof(typeParamImportRow) / sizeof(const type_info *);

    // Row adapter to select parameter_dic rows
    class ParamImportRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new ParamImportRow(); }
        int size(void) const { return sizeParamImportRow; }
        const type_info * const * columnTypes(void) const { return typeParamImportRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<ParamImportRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<ParamImportRow *>(i_row)->paramId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<ParamImportRow *>(i_row)->paramHid = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<ParamImportRow *>(i_row)->isFromParam = (*(bool *)i_value);
                break;
            case 4:
                dynamic_cast<ParamImportRow *>(i_row)->fromName = ((const char *)i_value);
                break;
            case 5:
                dynamic_cast<ParamImportRow *>(i_row)->fromModel = ((const char *)i_value);
                break;
            case 6:
                dynamic_cast<ParamImportRow *>(i_row)->isSampleDim = (*(bool *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IParamImportTable::~IParamImportTable(void) noexcept { }

// Create new table rows by loading db rows
IParamImportTable * IParamImportTable::create(IDbExec * i_dbExec, int i_modelId)
{
    return new ParamImportTable(i_dbExec, i_modelId);
}

// Create new table rows by swap with supplied vector of rows
IParamImportTable * IParamImportTable::create(IRowBaseVec & io_rowVec)
{
    return new ParamImportTable(io_rowVec);
}

// Load table
ParamImportTable::ParamImportTable(IDbExec * i_dbExec, int i_modelId)
{ 
    const IRowAdapter & adp = ParamImportRowAdapter();
    rowVec = load(
        "SELECT" \
        " I.model_id, I.model_parameter_id, D.parameter_hid, I.is_from_parameter, I.from_name, I.from_model_name, I.is_sample_dim" \
        " FROM parameter_dic D" \
        " INNER JOIN model_parameter_dic M ON (M.parameter_hid = D.parameter_hid)" \
        " INNER JOIN model_parameter_import I ON (I.model_id = M.model_id AND I.model_parameter_id = M.model_parameter_id)" +
        ((i_modelId > 0) ? " WHERE M.model_id = " + to_string(i_modelId) : "") +
        " ORDER BY 1, 2, 4, 5, 6", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
ParamImportTable::~ParamImportTable(void) noexcept { }

// Find row by unique key: model id, model parameter id, is_from_parameter, from_name, from_model_name
const ParamImportRow * ParamImportTable::byKey(int i_modelId, int i_paramId, bool i_isFromParam, const string & i_fromName, const string & i_fromModel) const
{
    const IRowBaseUptr keyRow( new ParamImportRow(i_modelId, i_paramId, i_isFromParam, i_fromName, i_fromModel) );
    return findKey(keyRow);
}

// get list of rows by model id
vector<ParamImportRow> ParamImportTable::byModelId(int i_modelId) const
{
    return findAll(
        [i_modelId](const ParamImportRow & i_row) -> bool { return i_row.modelId == i_modelId; }
    );
}
