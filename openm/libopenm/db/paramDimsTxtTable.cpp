// OpenM++ data library: parameter_dims_txt table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // parameter_dims_txt join to model_parameter_dic table implementation
    class ParamDimsTxtTable : public IParamDimsTxtTable
    {
    public:
        ParamDimsTxtTable(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);
        ParamDimsTxtTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~ParamDimsTxtTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec& rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) override { return rowVec; }

        // find row by unique key: model id,  model parameter id, dimension id, language id
        const ParamDimsTxtRow * byKey(int i_modelId, int i_paramId, int i_dimId, int i_langId) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        ParamDimsTxtTable(const ParamDimsTxtTable & i_table) = delete;
        ParamDimsTxtTable & operator=(const ParamDimsTxtTable & i_table) = delete;
    };

    // Columns type for parameter_dims_txt join to model_parameter_dic row
    static const type_info * typeParamDimsTxtRow[] = { 
        &typeid(decltype(ParamDimsTxtRow::modelId)), 
        &typeid(decltype(ParamDimsTxtRow::paramId)), 
        &typeid(decltype(ParamDimsTxtRow::dimId)), 
        &typeid(decltype(ParamDimsTxtRow::langId)), 
        &typeid(decltype(ParamDimsTxtRow::descr)), 
        &typeid(decltype(ParamDimsTxtRow::note)) 
    };

    // Size (number of columns) for parameter_dims_txt join to model_parameter_dic row
    static const int sizeParamDimsTxtRow = sizeof(typeParamDimsTxtRow) / sizeof(const type_info *);

    // Row adapter to select parameter_dims_txt join to model_parameter_dic rows
    class ParamDimsTxtRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new ParamDimsTxtRow(); }
        int size(void) const { return sizeParamDimsTxtRow; }
        const type_info * const * columnTypes(void) const { return typeParamDimsTxtRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<ParamDimsTxtRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<ParamDimsTxtRow *>(i_row)->paramId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<ParamDimsTxtRow *>(i_row)->dimId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<ParamDimsTxtRow *>(i_row)->langId = (*(int *)i_value);
                break;
            case 4:
                dynamic_cast<ParamDimsTxtRow *>(i_row)->descr = ((const char *)i_value);
                break;
            case 5:
                dynamic_cast<ParamDimsTxtRow *>(i_row)->note = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IParamDimsTxtTable::~IParamDimsTxtTable(void) noexcept { }

// Create new table rows by loading db rows
IParamDimsTxtTable * IParamDimsTxtTable::create(IDbExec * i_dbExec, int i_modelId, int i_langId)
{
    return new ParamDimsTxtTable(i_dbExec, i_modelId, i_langId);
}

// Create new table rows by swap with supplied vector of rows
IParamDimsTxtTable * IParamDimsTxtTable::create(IRowBaseVec & io_rowVec)
{
    return new ParamDimsTxtTable(io_rowVec);
}

// Load table
ParamDimsTxtTable::ParamDimsTxtTable(IDbExec * i_dbExec, int i_modelId, int i_langId)   
{ 
    string sWhere = "";
    if (i_modelId > 0 && i_langId < 0) sWhere = " WHERE M.model_id = " + to_string(i_modelId);
    if (i_modelId <= 0 && i_langId >= 0) sWhere = " WHERE D.lang_id = " + to_string(i_langId);
    if (i_modelId > 0 && i_langId >= 0) sWhere = " WHERE M.model_id = " + to_string(i_modelId) + " AND D.lang_id = " + to_string(i_langId);

    const IRowAdapter & adp = ParamDimsTxtRowAdapter();
    rowVec = load(
        "SELECT M.model_id, M.model_parameter_id, D.dim_id, D.lang_id, D.descr, D.note" \
        " FROM parameter_dims_txt D" \
        " INNER JOIN model_parameter_dic M ON (M.parameter_hid = D.parameter_hid)" +
        sWhere + 
        " ORDER BY 1, 2, 3, 4", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
ParamDimsTxtTable::~ParamDimsTxtTable(void) noexcept { }

// Find row by unique key: model id,  model parameter id, dimension id, language id
const ParamDimsTxtRow * ParamDimsTxtTable::byKey(int i_modelId, int i_paramId, int i_dimId, int i_langId) const
{
    const IRowBaseUptr keyRow( new ParamDimsTxtRow(i_modelId, i_paramId, i_dimId, i_langId) );
    return findKey(keyRow);
}

