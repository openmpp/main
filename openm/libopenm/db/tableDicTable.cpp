// OpenM++ data library: table_dic join to model_table_dic table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // table_dic join to model_table_dic table implementation
    class TableDicTable : public ITableDicTable
    {
    public:
        TableDicTable(IDbExec * i_dbExec, int i_modelId = 0);
        TableDicTable(IRowBaseVec & io_rowVec) { rowVec.swap(io_rowVec); }
        ~TableDicTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec& rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) override { return rowVec; }

        // find row by unique key: model id, model table id
        const TableDicRow * byKey(int i_modelId, int i_tableId) const override;

        // get list of rows by model id
        vector<TableDicRow> byModelId(int i_modelId) const override;

        // find first row by model id and table name or NULL if not found
        const TableDicRow * byModelIdName(int i_modelId, const string & i_name) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        TableDicTable(const TableDicTable & i_table) = delete;
        TableDicTable & operator=(const TableDicTable & i_table) = delete;
    };

    // Columns type for table_dic join to model_table_dic row
    static const type_info * typeTableDicRow[] = { 
        &typeid(decltype(TableDicRow::modelId)), 
        &typeid(decltype(TableDicRow::tableId)), 
        &typeid(decltype(TableDicRow::tableName)), 
        &typeid(decltype(TableDicRow::tableHid)), 
        &typeid(decltype(TableDicRow::digest)), 
        &typeid(decltype(TableDicRow::rank)), 
        &typeid(decltype(TableDicRow::isSparse)),
        &typeid(decltype(TableDicRow::isUser)), 
        &typeid(decltype(TableDicRow::dbExprTable)), 
        &typeid(decltype(TableDicRow::dbAccTable)), 
        &typeid(decltype(TableDicRow::dbAccAll)), 
        &typeid(decltype(TableDicRow::exprPos))
    };

    // Size (number of columns) for table_dic join to model_table_dic row
    static const int sizeTableDicRow = sizeof(typeTableDicRow) / sizeof(const type_info *);

    // Row adapter to select table_dic join to model_table_dic rows
    class TableDicRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new TableDicRow(); }
        int size(void) const { return sizeTableDicRow; }
        const type_info * const * columnTypes(void) const { return typeTableDicRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<TableDicRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<TableDicRow *>(i_row)->tableId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<TableDicRow *>(i_row)->tableName = ((const char *)i_value);
                break;
            case 3:
                dynamic_cast<TableDicRow *>(i_row)->tableHid = (*(int *)i_value);
                break;
            case 4:
                dynamic_cast<TableDicRow *>(i_row)->digest = ((const char *)i_value);
                break;
            case 5:
                dynamic_cast<TableDicRow *>(i_row)->rank = (*(int *)i_value);
                break;
            case 6:
                dynamic_cast<TableDicRow *>(i_row)->isSparse = (*(bool *)i_value);
                break;
            case 7:
                dynamic_cast<TableDicRow *>(i_row)->isUser = (*(bool *)i_value);
                break;
            case 8:
                dynamic_cast<TableDicRow *>(i_row)->dbExprTable = ((const char *)i_value);
                break;
            case 9:
                dynamic_cast<TableDicRow *>(i_row)->dbAccTable = ((const char *)i_value);
                break;
            case 10:
                dynamic_cast<TableDicRow *>(i_row)->dbAccAll = ((const char *)i_value);
                break;
            case 11:
                dynamic_cast<TableDicRow *>(i_row)->exprPos = (*(int *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITableDicTable::~ITableDicTable(void) noexcept { }

// Create new table rows by loading db rows
ITableDicTable * ITableDicTable::create(IDbExec * i_dbExec, int i_modelId)
{
    return new TableDicTable(i_dbExec, i_modelId);
}

// Create new table rows by swap with supplied vector of rows
ITableDicTable * ITableDicTable::create(IRowBaseVec & io_rowVec)
{
    return new TableDicTable(io_rowVec);
}

// Load table
TableDicTable::TableDicTable(IDbExec * i_dbExec, int i_modelId)   
{ 
    const IRowAdapter & adp = TableDicRowAdapter();
    rowVec = load(
        "SELECT" \
        " M.model_id, M.model_table_id, D.table_name, D.table_hid," \
        " D.table_digest, D.table_rank, D.is_sparse, M.is_user," \
        " D.db_expr_table, D.db_acc_table, D.db_acc_all_view, M.expr_dim_pos" \
        " FROM table_dic D" \
        " INNER JOIN model_table_dic M ON (M.table_hid = D.table_hid)" +
        ((i_modelId > 0) ? " WHERE M.model_id = " + to_string(i_modelId) : "") +
        " ORDER BY 1, 2", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
TableDicTable::~TableDicTable(void) noexcept { }

// Find row by unique key: model id, model table id
const TableDicRow * TableDicTable::byKey(int i_modelId, int i_tableId) const
{
    const IRowBaseUptr keyRow( new TableDicRow(i_modelId, i_tableId) );
    return findKey(keyRow);
}

// find first row by model id and table name or NULL if not found
const TableDicRow * TableDicTable::byModelIdName(int i_modelId, const string & i_name) const
{
    return findFirst(
        [i_modelId, i_name](const TableDicRow & i_row) -> bool {
                return i_row.modelId == i_modelId && i_row.tableName == i_name;
            }
        );
}

// get list of rows by model id
vector<TableDicRow> TableDicTable::byModelId(int i_modelId) const
{
    return findAll(
        [i_modelId](const TableDicRow & i_row) -> bool { return i_row.modelId == i_modelId; }
    );
}
