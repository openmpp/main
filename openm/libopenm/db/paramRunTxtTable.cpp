// OpenM++ data library: parameter_run_txt table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // parameter_run_txt table implementation
    class ParamRunTxtTable : public IParamRunTxtTable
    {
    public:
        ParamRunTxtTable(IDbExec * i_dbExec, int i_runId = 0, int i_langId = -1);
        ParamRunTxtTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~ParamRunTxtTable() throw();

        // get const reference to list of all table rows
        const IRowBaseVec & rowsCRef(void) const { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by primary key: run id, parameter id, language id
        const ParamRunTxtRow * byKey(int i_runId, int i_paramId, int i_langId) const;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        ParamRunTxtTable(const ParamRunTxtTable & i_table) = delete;
        ParamRunTxtTable & operator=(const ParamRunTxtTable & i_table) = delete;
    };

    // Columns type for parameter_run_txt row
    static const type_info * typeParamRunTxtRow[] = { 
        &typeid(decltype(ParamRunTxtRow::runId)), 
        &typeid(decltype(ParamRunTxtRow::modelId)), 
        &typeid(decltype(ParamRunTxtRow::paramId)), 
        &typeid(decltype(ParamRunTxtRow::langId)), 
        &typeid(decltype(ParamRunTxtRow::note))
    };

    // Size (number of columns) for parameter_run_txt row
    static const int sizeParamRunTxtRow = sizeof(typeParamRunTxtRow) / sizeof(const type_info *);

    // Row adapter to select parameter_run_txt rows
    class ParamRunTxtRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new ParamRunTxtRow(); }
        int size(void) const { return sizeParamRunTxtRow; }
        const type_info ** columnTypes(void) const { return typeParamRunTxtRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<ParamRunTxtRow *>(i_row)->runId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<ParamRunTxtRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<ParamRunTxtRow *>(i_row)->paramId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<ParamRunTxtRow *>(i_row)->langId = (*(int *)i_value);
                break;
            case 4:
                dynamic_cast<ParamRunTxtRow *>(i_row)->note = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IParamRunTxtTable::~IParamRunTxtTable(void) throw() { }

// Create new table rows by loading db rows
IParamRunTxtTable * IParamRunTxtTable::create(IDbExec * i_dbExec, int i_runId, int i_langId)
{
    return new ParamRunTxtTable(i_dbExec, i_runId, i_langId);
}

// Create new table rows by swap with supplied vector of rows
IParamRunTxtTable * IParamRunTxtTable::create(IRowBaseVec & io_rowVec)
{
    return new ParamRunTxtTable(io_rowVec);
}

// Load table
ParamRunTxtTable::ParamRunTxtTable(IDbExec * i_dbExec, int i_runId, int i_langId)   
{ 
    string sWhere = "";
    if (i_runId > 0 && i_langId < 0) sWhere = " WHERE run_id = " + to_string(i_runId);
    if (i_runId <= 0 && i_langId >= 0) sWhere = " WHERE lang_id = " + to_string(i_langId);
    if (i_runId > 0 && i_langId >= 0) sWhere = " WHERE run_id = " + to_string(i_runId) + " AND lang_id = " + to_string(i_langId);

    const IRowAdapter & adp = ParamRunTxtRowAdapter();
    rowVec = load(
        "SELECT run_id, model_id, parameter_id, lang_id, note FROM parameter_run_txt" + sWhere + " ORDER BY 1, 3, 4", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
ParamRunTxtTable::~ParamRunTxtTable(void) throw() { }

// Find row by primary key: run id, parameter id, language id
const ParamRunTxtRow * ParamRunTxtTable::byKey(int i_runId, int i_paramId, int i_langId) const
{
    const IRowBaseUptr keyRow( new ParamRunTxtRow(i_runId, i_paramId, i_langId) );
    return findKey(keyRow);
}
