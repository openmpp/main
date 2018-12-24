// OpenM++ data library: parameter_dic_txt join to model_parameter_dic table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // parameter_dic_txt join to model_parameter_dic table implementation
    class ParamDicTxtTable : public IParamDicTxtTable
    {
    public:
        ParamDicTxtTable(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);
        ParamDicTxtTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~ParamDicTxtTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec & rowsCRef(void) const { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by unique key: model id,  model parameter id, language id
        const ParamDicTxtRow * byKey(int i_modelId, int i_paramId, int i_langId) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        ParamDicTxtTable(const ParamDicTxtTable & i_table) = delete;
        ParamDicTxtTable & operator=(const ParamDicTxtTable & i_table) = delete;
    };

    // Columns type for parameter_dic_txt join to model_parameter_dic row
    static const type_info * typeParamDicTxtRow[] = { 
        &typeid(decltype(ParamDicTxtRow::modelId)), 
        &typeid(decltype(ParamDicTxtRow::paramId)), 
        &typeid(decltype(ParamDicTxtRow::langId)), 
        &typeid(decltype(ParamDicTxtRow::descr)), 
        &typeid(decltype(ParamDicTxtRow::note))
    };

    // Size (number of columns) for parameter_dic_txt join to model_parameter_dic row
    static const int sizeParamDicTxtRow = sizeof(typeParamDicTxtRow) / sizeof(const type_info *);

    // Row adapter to select parameter_dic_txt join to model_parameter_dic rows
    class ParamDicTxtRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new ParamDicTxtRow(); }
        int size(void) const { return sizeParamDicTxtRow; }
        const type_info * const * columnTypes(void) const { return typeParamDicTxtRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<ParamDicTxtRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<ParamDicTxtRow *>(i_row)->paramId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<ParamDicTxtRow *>(i_row)->langId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<ParamDicTxtRow *>(i_row)->descr = ((const char *)i_value);
                break;
            case 4:
                dynamic_cast<ParamDicTxtRow *>(i_row)->note = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IParamDicTxtTable::~IParamDicTxtTable(void) noexcept { }

// Create new table rows by loading db rows
IParamDicTxtTable * IParamDicTxtTable::create(IDbExec * i_dbExec, int i_modelId, int i_langId)
{
    return new ParamDicTxtTable(i_dbExec, i_modelId, i_langId);
}

// Create new table rows by swap with supplied vector of rows
IParamDicTxtTable * IParamDicTxtTable::create(IRowBaseVec & io_rowVec)
{
    return new ParamDicTxtTable(io_rowVec);
}

// Load table
ParamDicTxtTable::ParamDicTxtTable(IDbExec * i_dbExec, int i_modelId, int i_langId)   
{ 
    string sWhere = "";
    if (i_modelId > 0 && i_langId < 0) sWhere = " WHERE M.model_id = " + to_string(i_modelId);
    if (i_modelId <= 0 && i_langId >= 0) sWhere = " WHERE D.lang_id = " + to_string(i_langId);
    if (i_modelId > 0 && i_langId >= 0) sWhere = " WHERE M.model_id = " + to_string(i_modelId) + " AND D.lang_id = " + to_string(i_langId);

    const IRowAdapter & adp = ParamDicTxtRowAdapter();
    rowVec = load(
        "SELECT M.model_id, M.model_parameter_id, D.lang_id, D.descr, D.note" \
        " FROM parameter_dic_txt D" \
        " INNER JOIN model_parameter_dic M ON (M.parameter_hid = D.parameter_hid)" +
        sWhere + 
        " ORDER BY 1, 2, 3", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
ParamDicTxtTable::~ParamDicTxtTable(void) noexcept { }

// Find row by unique key: model id,  model parameter id, language id
const ParamDicTxtRow * ParamDicTxtTable::byKey(int i_modelId, int i_paramId, int i_langId) const
{
    const IRowBaseUptr keyRow( new ParamDicTxtRow(i_modelId, i_paramId, i_langId) );
    return findKey(keyRow);
}

