// OpenM++ data library: table_expr table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // table_expr table implementation
    class TableExprTable : public ITableExprTable
    {
    public:
        TableExprTable(IDbExec * i_dbExec, int i_modelId = 0);
        TableExprTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~TableExprTable() throw();

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by primary key: model id, table id, expr id
        const TableExprRow * byKey(int i_modelId, int i_tableId, int i_exprId) const;

        // get list of all table rows
        vector<TableExprRow> rows(void) const { return IMetaTable<TableExprRow>::rows(rowVec); }

        // get list of rows by model id
        vector<TableExprRow> byModelId(int i_modelId) const;

        // get list of rows by model id and table id
        vector<TableExprRow> byModelIdTableId(int i_modelId, int i_tableId) const;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        TableExprTable(const TableExprTable & i_table);             // = delete;
        TableExprTable & operator=(const TableExprTable & i_table); // = delete;
    };

    // Columns type for table_expr row
    static const type_info * typeTableExprRow[] = { 
        &typeid(decltype(TableExprRow::modelId)), 
        &typeid(decltype(TableExprRow::tableId)), 
        &typeid(decltype(TableExprRow::exprId)), 
        &typeid(decltype(TableExprRow::name)), 
        &typeid(decltype(TableExprRow::decimals)),
        &typeid(decltype(TableExprRow::src)), 
        &typeid(decltype(TableExprRow::expr)) 
    };

    // Size (number of columns) for table_expr row
    static const int sizeTableExprRow = sizeof(typeTableExprRow) / sizeof(const type_info *);

    // Row adapter to select table_expr rows
    class TableExprRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new TableExprRow(); }
        int size(void) const { return sizeTableExprRow; }
        const type_info ** columnTypes(void) const { return typeTableExprRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<TableExprRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<TableExprRow *>(i_row)->tableId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<TableExprRow *>(i_row)->exprId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<TableExprRow *>(i_row)->name = ((const char *)i_value);
                break;
            case 4:
                dynamic_cast<TableExprRow *>(i_row)->decimals = (*(int *)i_value);
                break;
            case 5:
                dynamic_cast<TableExprRow *>(i_row)->src = ((const char *)i_value);
                break;
            case 6:
                dynamic_cast<TableExprRow *>(i_row)->expr = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITableExprTable::~ITableExprTable(void) throw() { }

// Create new table rows by loading db rows
ITableExprTable * ITableExprTable::create(IDbExec * i_dbExec, int i_modelId)
{
    return new TableExprTable(i_dbExec, i_modelId);
}

// Create new table rows by swap with supplied vector of rows
ITableExprTable * ITableExprTable::create(IRowBaseVec & io_rowVec)
{
    return new TableExprTable(io_rowVec);
}

// Load table
TableExprTable::TableExprTable(IDbExec * i_dbExec, int i_modelId)
{ 
    const IRowAdapter & adp = TableExprRowAdapter();
    rowVec = IMetaTable<TableExprRow>::load(
        "SELECT" \
        " model_id, table_id, expr_id, expr_name, expr_decimals, expr_src, expr_sql" \
        " FROM table_expr" + 
        ((i_modelId > 0) ? " WHERE model_id = " + to_string(i_modelId) : "") +
        " ORDER BY 1, 2, 3", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
TableExprTable::~TableExprTable(void) throw() { }

// Find row by primary key: model id, table id, expr id
const TableExprRow * TableExprTable::byKey(int i_modelId, int i_tableId, int i_exprId) const
{
    const IRowBaseUptr keyRow( new TableExprRow(i_modelId, i_tableId, i_exprId) );
    return IMetaTable<TableExprRow>::byKey(keyRow, rowVec);
}

// get list of rows by model id
vector<TableExprRow> TableExprTable::byModelId(int i_modelId) const
{
    const IRowBaseUptr row(new TableExprRow(i_modelId, 0, 0));
    return IMetaTable<TableExprRow>::findAll(row, rowVec, TableExprRow::modelIdEqual);
}

// get list of rows by model id and table id
vector<TableExprRow> TableExprTable::byModelIdTableId(int i_modelId, int i_tableId) const
{
    const IRowBaseUptr row( new TableExprRow(i_modelId, i_tableId, 0));
    return IMetaTable<TableExprRow>::findAll(row, rowVec, TableExprRow::modelIdTableIdEqual);
}
