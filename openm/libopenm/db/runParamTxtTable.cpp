// OpenM++ data library: run_parameter_txt join to model_parameter_dic table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // run_parameter_txt join to model_parameter_dic table implementation
    class RunParamTxtTable : public IRunParamTxtTable
    {
    public:
        RunParamTxtTable(IDbExec * i_dbExec, int i_runId = 0, int i_langId = -1);
        RunParamTxtTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~RunParamTxtTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec& rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) override { return rowVec; }

        // find row by primary key: run id, parameter id, language id
        const RunParamTxtRow * byKey(int i_runId, int i_paramId, int i_langId) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        RunParamTxtTable(const RunParamTxtTable & i_table) = delete;
        RunParamTxtTable & operator=(const RunParamTxtTable & i_table) = delete;
    };

    // Columns type for run_parameter_txt join to model_parameter_dic row
    static const type_info * typeRunParamTxtRow[] = { 
        &typeid(decltype(RunParamTxtRow::runId)), 
        &typeid(decltype(RunParamTxtRow::modelId)), 
        &typeid(decltype(RunParamTxtRow::paramId)), 
        &typeid(decltype(RunParamTxtRow::langId)), 
        &typeid(decltype(RunParamTxtRow::note))
    };

    // Size (number of columns) for run_parameter_txt join to model_parameter_dic row
    static const int sizeRunParamTxtRow = sizeof(typeRunParamTxtRow) / sizeof(const type_info *);

    // Row adapter to select run_parameter_txt rows
    class RunParamTxtRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new RunParamTxtRow(); }
        int size(void) const { return sizeRunParamTxtRow; }
        const type_info * const * columnTypes(void) const { return typeRunParamTxtRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<RunParamTxtRow *>(i_row)->runId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<RunParamTxtRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<RunParamTxtRow *>(i_row)->paramId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<RunParamTxtRow *>(i_row)->langId = (*(int *)i_value);
                break;
            case 4:
                dynamic_cast<RunParamTxtRow *>(i_row)->note = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IRunParamTxtTable::~IRunParamTxtTable(void) noexcept { }

// Create new table rows by loading db rows
IRunParamTxtTable * IRunParamTxtTable::create(IDbExec * i_dbExec, int i_runId, int i_langId)
{
    return new RunParamTxtTable(i_dbExec, i_runId, i_langId);
}

// Create new table rows by swap with supplied vector of rows
IRunParamTxtTable * IRunParamTxtTable::create(IRowBaseVec & io_rowVec)
{
    return new RunParamTxtTable(io_rowVec);
}

// Load table
RunParamTxtTable::RunParamTxtTable(IDbExec * i_dbExec, int i_runId, int i_langId)   
{ 
    string sWhere = "";
    if (i_runId > 0 && i_langId < 0) sWhere = " WHERE T.run_id = " + to_string(i_runId);
    if (i_runId <= 0 && i_langId >= 0) sWhere = " WHERE T.lang_id = " + to_string(i_langId);
    if (i_runId > 0 && i_langId >= 0) sWhere = " WHERE T.run_id = " + to_string(i_runId) + " AND T.lang_id = " + to_string(i_langId);

    const IRowAdapter & adp = RunParamTxtRowAdapter();
    rowVec = load(
        "SELECT T.run_id, M.model_id, M.model_parameter_id, T.lang_id, T.note" \
        " FROM run_parameter_txt T" \
        " INNER JOIN model_parameter_dic M ON (M.parameter_hid = T.parameter_hid)" +
        sWhere + 
        " ORDER BY 1, 3, 4", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
RunParamTxtTable::~RunParamTxtTable(void) noexcept { }

// Find row by primary key: run id, parameter id, language id
const RunParamTxtRow * RunParamTxtTable::byKey(int i_runId, int i_paramId, int i_langId) const
{
    const IRowBaseUptr keyRow( new RunParamTxtRow(i_runId, i_paramId, i_langId) );
    return findKey(keyRow);
}
