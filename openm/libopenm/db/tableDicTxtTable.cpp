// OpenM++ data library: table_dic_txt table
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // table_dic_txt table implementation
    class TableDicTxtTable : public ITableDicTxtTable
    {
    public:
        TableDicTxtTable(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);
        TableDicTxtTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~TableDicTxtTable() throw();

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by primary key: model id, table id, language id
        const TableDicTxtRow * byKey(int i_modelId, int i_tableId, int i_langId) const;

        // get list of all table rows
        vector<TableDicTxtRow> rows(void) const { return IMetaTable<TableDicTxtRow>::rows(rowVec); }

        // get list of rows by language
        vector<TableDicTxtRow> byLang(int i_langId) const;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        TableDicTxtTable(const TableDicTxtTable & i_table);               // = delete;
        TableDicTxtTable & operator=(const TableDicTxtTable & i_table);   // = delete;
    };

    // Columns type for table_dic_txt row
    static const type_info * typeTableDicTxtRow[] = { 
        &typeid(decltype(TableDicTxtRow::modelId)), 
        &typeid(decltype(TableDicTxtRow::tableId)), 
        &typeid(decltype(TableDicTxtRow::langId)), 
        &typeid(decltype(TableDicTxtRow::descr)), 
        &typeid(decltype(TableDicTxtRow::note)), 
        &typeid(decltype(TableDicTxtRow::unitDescr)), 
        &typeid(decltype(TableDicTxtRow::unitNote))
    };

    // Size (number of columns) for table_dic_txt row
    static const int sizeTableDicTxtRow = sizeof(typeTableDicTxtRow) / sizeof(const type_info *);

    // Row adapter to select table_dic_txt rows
    class TableDicTxtRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new TableDicTxtRow(); }
        int size(void) const { return sizeTableDicTxtRow; }
        const type_info ** columnTypes(void) const { return typeTableDicTxtRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<TableDicTxtRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<TableDicTxtRow *>(i_row)->tableId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<TableDicTxtRow *>(i_row)->langId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<TableDicTxtRow *>(i_row)->descr = ((const char *)i_value);
                break;
            case 4:
                dynamic_cast<TableDicTxtRow *>(i_row)->note = ((const char *)i_value);
                break;
            case 5:
                dynamic_cast<TableDicTxtRow *>(i_row)->unitDescr = ((const char *)i_value);
                break;
            case 6:
                dynamic_cast<TableDicTxtRow *>(i_row)->unitNote = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITableDicTxtTable::~ITableDicTxtTable(void) throw() { }

// Create new table rows by loading db rows
ITableDicTxtTable * ITableDicTxtTable::create(IDbExec * i_dbExec, int i_modelId, int i_langId)
{
    return new TableDicTxtTable(i_dbExec, i_modelId, i_langId);
}

// Create new table rows by swap with supplied vector of rows
ITableDicTxtTable * ITableDicTxtTable::create(IRowBaseVec & io_rowVec)
{
    return new TableDicTxtTable(io_rowVec);
}

// Load table
TableDicTxtTable::TableDicTxtTable(IDbExec * i_dbExec, int i_modelId, int i_langId)
{ 
    string sWhere = "";
    if (i_modelId > 0 && i_langId < 0) sWhere = " WHERE model_id = " + to_string(i_modelId);
    if (i_modelId <= 0 && i_langId >= 0) sWhere = " WHERE lang_id = " + to_string(i_langId);
    if (i_modelId > 0 && i_langId >= 0) sWhere = " WHERE model_id = " + to_string(i_modelId) + " AND lang_id = " + to_string(i_langId);

    const IRowAdapter & adp = TableDicTxtRowAdapter();
    rowVec = IMetaTable<TableDicTxtRow>::load(
        "SELECT model_id, table_id, lang_id, descr, note, unit_descr, unit_note FROM table_dic_txt" +  sWhere + " ORDER BY 1, 2, 3", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
TableDicTxtTable::~TableDicTxtTable(void) throw() { }

// Find row by primary key: model id, table id, language id
const TableDicTxtRow * TableDicTxtTable::byKey(int i_modelId, int i_tableId, int i_langId) const
{
    const IRowBaseUptr keyRow( new TableDicTxtRow(i_modelId, i_tableId, i_langId) );
    return IMetaTable<TableDicTxtRow>::byKey(keyRow, rowVec);
}

// get list of rows by language
vector<TableDicTxtRow> TableDicTxtTable::byLang(int i_langId) const
{
    const IRowBaseUptr row( new TableDicTxtRow(0, 0, i_langId) );
    return IMetaTable<TableDicTxtRow>::findAll(row, rowVec, TableDicTxtRow::langEqual);
}
