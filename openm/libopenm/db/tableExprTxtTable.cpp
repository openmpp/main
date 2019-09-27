// OpenM++ data library: table_expr_txt join to model_table_dic table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // table_expr_txt join to model_table_dic table implementation
    class TableExprTxtTable : public ITableExprTxtTable
    {
    public:
        TableExprTxtTable(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);
        TableExprTxtTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~TableExprTxtTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec& rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by unique key: model id, model table id, expr id, language id
        const TableExprTxtRow * byKey(int i_modelId, int i_tableId, int i_exprId, int i_langId) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        TableExprTxtTable(const TableExprTxtTable & i_table) = delete;
        TableExprTxtTable & operator=(const TableExprTxtTable & i_table) = delete;
    };

    // Columns type for table_expr_txt join to model_table_dic row
    static const type_info * typeTableExprTxtRow[] = { 
        &typeid(decltype(TableExprTxtRow::modelId)), 
        &typeid(decltype(TableExprTxtRow::tableId)), 
        &typeid(decltype(TableExprTxtRow::exprId)), 
        &typeid(decltype(TableExprTxtRow::langId)), 
        &typeid(decltype(TableExprTxtRow::descr)), 
        &typeid(decltype(TableExprTxtRow::note)) 
    };

    // Size (number of columns) for table_expr_txt join to model_table_dic row
    static const int sizeTableExprTxtRow = sizeof(typeTableExprTxtRow) / sizeof(const type_info *);

    // Row adapter to select table_expr_txt join to model_table_dic rows
    class TableExprTxtRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new TableExprTxtRow(); }
        int size(void) const { return sizeTableExprTxtRow; }
        const type_info * const * columnTypes(void) const { return typeTableExprTxtRow; }

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
ITableExprTxtTable::~ITableExprTxtTable(void) noexcept { }

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
    rowVec = load(
        "SELECT M.model_id, M.model_table_id, D.expr_id, D.lang_id, D.descr, D.note" \
        " FROM table_expr_txt D" \
        " INNER JOIN model_table_dic M ON (M.table_hid = D.table_hid)" +
        sWhere + 
        " ORDER BY 1, 2, 3, 4", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
TableExprTxtTable::~TableExprTxtTable(void) noexcept { }

// Find row by unique key: model id, model table id, expr id, language id
const TableExprTxtRow * TableExprTxtTable::byKey(int i_modelId, int i_tableId, int i_exprId, int i_langId) const
{
    const IRowBaseUptr keyRow( new TableExprTxtRow(i_modelId, i_tableId, i_exprId, i_langId) );
    return findKey(keyRow);
}
