// OpenM++ data library: table_expr_txt table
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // table_expr_txt table implementation
    class TableExprTxtTable : public ITableExprTxtTable
    {
    public:
        TableExprTxtTable(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);
        TableExprTxtTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~TableExprTxtTable() throw();

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by primary key: model id, table id, expr id, language id
        const TableExprTxtRow * byKey(int i_modelId, int i_tableId, int i_exprId, int i_langId) const;

        // get list of all table rows
        vector<TableExprTxtRow> rows(void) const { return IMetaTable<TableExprTxtRow>::rows(rowVec); }

        // get list of rows by language
        vector<TableExprTxtRow> byLang(int i_langId) const;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        TableExprTxtTable(const TableExprTxtTable & i_table);               // = delete;
        TableExprTxtTable & operator=(const TableExprTxtTable & i_table);   // = delete;
    };

    // Columns type for table_expr_txt row
    static const type_info * typeTableExprTxtRow[] = { 
        &typeid(decltype(TableExprTxtRow::modelId)), 
        &typeid(decltype(TableExprTxtRow::tableId)), 
        &typeid(decltype(TableExprTxtRow::exprId)), 
        &typeid(decltype(TableExprTxtRow::langId)), 
        &typeid(decltype(TableExprTxtRow::descr)), 
        &typeid(decltype(TableExprTxtRow::note)) 
    };

    // Size (number of columns) for table_expr_txt row
    static const int sizeTableExprTxtRow = sizeof(typeTableExprTxtRow) / sizeof(const type_info *);

    // Row adapter to select table_expr_txt rows
    class TableExprTxtRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new TableExprTxtRow(); }
        int size(void) const { return sizeTableExprTxtRow; }
        const type_info ** columnTypes(void) const { return typeTableExprTxtRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<TableExprTxtRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<TableExprTxtRow *>(i_row)->tableId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<TableExprTxtRow *>(i_row)->exprId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<TableExprTxtRow *>(i_row)->langId = (*(int *)i_value);
                break;
            case 4:
                dynamic_cast<TableExprTxtRow *>(i_row)->descr = ((const char *)i_value);
                break;
            case 5:
                dynamic_cast<TableExprTxtRow *>(i_row)->note = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITableExprTxtTable::~ITableExprTxtTable(void) throw() { }

// Create new table rows by loading db rows
ITableExprTxtTable * ITableExprTxtTable::create(IDbExec * i_dbExec, int i_modelId, int i_langId)
{
    return new TableExprTxtTable(i_dbExec, i_modelId, i_langId);
}

// Create new table rows by swap with supplied vector of rows
ITableExprTxtTable * ITableExprTxtTable::create(IRowBaseVec & io_rowVec)
{
    return new TableExprTxtTable(io_rowVec);
}

// Load table
TableExprTxtTable::TableExprTxtTable(IDbExec * i_dbExec, int i_modelId, int i_langId)
{ 
    string sWhere = "";
    if (i_modelId > 0 && i_langId < 0) sWhere = " WHERE model_id = " + to_string(i_modelId);
    if (i_modelId <= 0 && i_langId >= 0) sWhere = " WHERE lang_id = " + to_string(i_langId);
    if (i_modelId > 0 && i_langId >= 0) sWhere = " WHERE model_id = " + to_string(i_modelId) + " AND lang_id = " + to_string(i_langId);

    const IRowAdapter & adp = TableExprTxtRowAdapter();
    rowVec = IMetaTable<TableExprTxtRow>::load(
        "SELECT model_id, table_id, expr_id, lang_id, descr, note FROM table_expr_txt" +  sWhere + " ORDER BY 1, 2, 3, 4", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
TableExprTxtTable::~TableExprTxtTable(void) throw() { }

// Find row by primary key: model id, table id, expr id, language id
const TableExprTxtRow * TableExprTxtTable::byKey(int i_modelId, int i_tableId, int i_exprId, int i_langId) const
{
    const IRowBaseUptr keyRow( new TableExprTxtRow(i_modelId, i_tableId, i_exprId, i_langId) );
    return IMetaTable<TableExprTxtRow>::byKey(keyRow, rowVec);
}

// get list of rows by language
vector<TableExprTxtRow> TableExprTxtTable::byLang(int i_langId) const
{
    const IRowBaseUptr row( new TableExprTxtRow(0, 0, 0, i_langId) );
    return IMetaTable<TableExprTxtRow>::findAll(row, rowVec, TableExprTxtRow::langEqual);
}
