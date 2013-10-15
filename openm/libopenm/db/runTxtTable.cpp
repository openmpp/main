// OpenM++ data library: run_txt table
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // run_txt table implementation
    class RunTxtTable : public IRunTxtTable
    {
    public:
        ~RunTxtTable() throw();

        // select table rows by specified filter, sorted by primary key: run id and language id
        static vector<RunTxtRow> select(IDbExec * i_dbExec, const string & i_where);

    private:
        RunTxtTable(const RunTxtTable & i_table);               // = delete;
        RunTxtTable & operator=(const RunTxtTable & i_table);   // = delete;
    };

    // Columns type for run_txt row
    static const type_info * typeRunTxtRow[] = { 
        &typeid(decltype(RunTxtRow::runId)), 
        &typeid(decltype(RunTxtRow::modelId)), 
        &typeid(decltype(RunTxtRow::langId)), 
        &typeid(decltype(RunTxtRow::descr)),
        &typeid(decltype(RunTxtRow::note))
    };

    // Size (number of columns) for run_txt row
    static const int sizeRunTxtRow = sizeof(typeRunTxtRow) / sizeof(const type_info *);

    // Row adapter to select run_txt rows
    class RunTxtRowAdapter : public IRowAdapter
    {
    public:
        RunTxtRowAdapter() { }

        IRowBase * createRow(void) const { return new RunTxtRow(); }
        int size(void) const { return sizeRunTxtRow; }
        const type_info ** columnTypes(void) const { return typeRunTxtRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<RunTxtRow *>(i_row)->runId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<RunTxtRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<RunTxtRow *>(i_row)->langId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<RunTxtRow *>(i_row)->descr = ((const char *)i_value);
                break;
            case 4:
                dynamic_cast<RunTxtRow *>(i_row)->note = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IRunTxtTable::~IRunTxtTable(void) throw() { }

// Table never unloaded
RunTxtTable::~RunTxtTable(void) throw() { }

// select table rows
vector<RunTxtRow> IRunTxtTable::select(IDbExec * i_dbExec, int i_modelId, int i_langId)
{
    string sWhere = "";
    if (i_modelId > 0 && i_langId < 0) sWhere = " WHERE model_id = " + to_string(i_modelId);
    if (i_modelId <= 0 && i_langId >= 0) sWhere = " WHERE lang_id = " + to_string(i_langId);
    if (i_modelId > 0 && i_langId >= 0) sWhere = " WHERE model_id = " + to_string(i_modelId) + " AND lang_id = " + to_string(i_langId);

    return 
        RunTxtTable::select(i_dbExec, sWhere);
}

// select table rows by run id and language id
vector<RunTxtRow> IRunTxtTable::byKey(IDbExec * i_dbExec, int i_runId, int i_langId)
{
    string sWhere = " WHERE run_id = " + to_string(i_runId) + " AND lang_id = " + to_string(i_langId);
    return 
        RunTxtTable::select(i_dbExec, sWhere);
}

// select table rows by specified filter
vector<RunTxtRow> RunTxtTable::select(IDbExec * i_dbExec, const string & i_where)
{
    if (i_dbExec == NULL) throw DbException("invalid (NULL) database connection");

    const IRowAdapter & adp = RunTxtRowAdapter();
    IRowBaseVec vec = i_dbExec->selectRowList(
        "SELECT run_id, model_id, lang_id, descr, note FROM run_txt " + i_where + " ORDER BY 1, 3", 
        adp
        );
    stable_sort(vec.begin(), vec.end(), RunTxtRow::keyLess);

    return IMetaTable<RunTxtRow>::rows(vec);
}
