// OpenM++ data library: parameter_dims join to model_parameter_dic table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // parameter_dims join to model_parameter_dic table implementation
    class ParamDimsTable : public IParamDimsTable
    {
    public:
        ParamDimsTable(IDbExec * i_dbExec, int i_modelId = 0);
        ParamDimsTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~ParamDimsTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec& rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) override { return rowVec; }

        // find row by unique key: model id,  model parameter id, dimension id
        const ParamDimsRow * byKey(int i_modelId, int i_paramId, int i_dimId) const override;

        // get list of rows by model id and parameter id
        vector<ParamDimsRow> byModelIdParamId(int i_modelId, int i_paramId) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        ParamDimsTable(const ParamDimsTable & i_table) = delete;
        ParamDimsTable & operator=(const ParamDimsTable & i_table) = delete;
    };

    // Columns type for parameter_dims join to model_parameter_dic row
    static const type_info * typeParamDimsRow[] = { 
        &typeid(decltype(ParamDimsRow::modelId)),
        &typeid(decltype(ParamDimsRow::paramId)),
        &typeid(decltype(ParamDimsRow::dimId)),
        &typeid(decltype(ParamDimsRow::name)),
        &typeid(decltype(ParamDimsRow::typeId))
    };

    // Size (number of columns) for parameter_dims join to model_parameter_dic row
    static const int sizeParamDimsRow = sizeof(typeParamDimsRow) / sizeof(const type_info *);

    // Row adapter to select parameter_dims join to model_parameter_dic rows
    class ParamDimsRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new ParamDimsRow(); }
        int size(void) const { return sizeParamDimsRow; }
        const type_info * const * columnTypes(void) const { return typeParamDimsRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<ParamDimsRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<ParamDimsRow *>(i_row)->paramId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<ParamDimsRow *>(i_row)->dimId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<ParamDimsRow *>(i_row)->name = ((const char *)i_value);
                break;
            case 4:
                dynamic_cast<ParamDimsRow *>(i_row)->typeId = (*(int *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IParamDimsTable::~IParamDimsTable(void) noexcept { }

// Create new table rows by loading db rows
IParamDimsTable * IParamDimsTable::create(IDbExec * i_dbExec, int i_modelId)
{
    return new ParamDimsTable(i_dbExec, i_modelId);
}

// Create new table rows by swap with supplied vector of rows
IParamDimsTable * IParamDimsTable::create(IRowBaseVec & io_rowVec)
{
    return new ParamDimsTable(io_rowVec);
}

// Load table
ParamDimsTable::ParamDimsTable(IDbExec * i_dbExec, int i_modelId)
{ 
    const IRowAdapter & adp = ParamDimsRowAdapter();
    rowVec = load(
        "SELECT" \
        " M.model_id, M.model_parameter_id, dim_id, dim_name, T.model_type_id" \
        " FROM parameter_dims D" \
        " INNER JOIN model_parameter_dic M ON (M.parameter_hid = D.parameter_hid)" \
        " INNER JOIN model_type_dic T ON (T.type_hid = D.type_hid)" +
        ((i_modelId > 0) ? " WHERE M.model_id = " + to_string(i_modelId) : "") +
        " ORDER BY 1, 2, 3", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
ParamDimsTable::~ParamDimsTable(void) noexcept { }

// Find row by unique key: model id,  model parameter id, dimension id
const ParamDimsRow * ParamDimsTable::byKey(int i_modelId, int i_paramId, int i_dimId) const
{
    const IRowBaseUptr keyRow(new ParamDimsRow(i_modelId, i_paramId, i_dimId));
    return findKey(keyRow);
}

// get list of rows by model id and parameter id
vector<ParamDimsRow> ParamDimsTable::byModelIdParamId(int i_modelId, int i_paramId) const
{
    return findAll(
        [i_modelId, i_paramId](const ParamDimsRow & i_row) -> bool {
            return i_row.modelId == i_modelId && i_row.paramId == i_paramId;
        }
    );
}
