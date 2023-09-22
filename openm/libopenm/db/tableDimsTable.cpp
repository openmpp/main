// OpenM++ data library: table_dims join to model_table_dic table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // table_dims join to model_table_dic table implementation
    class TableDimsTable : public ITableDimsTable
    {
    public:
        TableDimsTable(IDbExec * i_dbExec, int i_modelId = 0);
        TableDimsTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~TableDimsTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec& rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) override { return rowVec; }

        // find row by unique key: model id, model table id, dimension id
        const TableDimsRow * byKey(int i_modelId, int i_tableId, int i_dimId) const override;

        // get list of rows by model id and table id
        vector<TableDimsRow> byModelIdTableId(int i_modelId, int i_tableId) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        TableDimsTable(const TableDimsTable & i_table) = delete;
        TableDimsTable & operator=(const TableDimsTable & i_table) = delete;
    };

    // Columns type for table_dims join to model_table_dic row
    static const type_info * typeTableDimsRow[] = { 
        &typeid(decltype(TableDimsRow::modelId)),
        &typeid(decltype(TableDimsRow::tableId)),
        &typeid(decltype(TableDimsRow::dimId)),
        &typeid(decltype(TableDimsRow::name)),
        &typeid(decltype(TableDimsRow::typeId)),
        &typeid(decltype(TableDimsRow::isTotal)),
        &typeid(decltype(TableDimsRow::dimSize))
    };

    // Size (number of columns) for table_dims join to model_table_dic row
    static const int sizeTableDimsRow = sizeof(typeTableDimsRow) / sizeof(const type_info *);

    // Row adapter to select table_dims join to model_table_dic rows
    class TableDimsRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new TableDimsRow(); }
        int size(void) const { return sizeTableDimsRow; }
        const type_info * const * columnTypes(void) const { return typeTableDimsRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<TableDimsRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<TableDimsRow *>(i_row)->tableId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<TableDimsRow *>(i_row)->dimId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<TableDimsRow *>(i_row)->name = ((const char *)i_value);
                break;
            case 4:
                dynamic_cast<TableDimsRow *>(i_row)->typeId = (*(int *)i_value);
                break;
            case 5:
                dynamic_cast<TableDimsRow *>(i_row)->isTotal = (*(bool *)i_value);
                break;
            case 6:
                dynamic_cast<TableDimsRow *>(i_row)->dimSize = (*(int *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITableDimsTable::~ITableDimsTable(void) noexcept { }

// Create new table rows by loading db rows
ITableDimsTable * ITableDimsTable::create(IDbExec * i_dbExec, int i_modelId)
{
    return new TableDimsTable(i_dbExec, i_modelId);
}

// Create new table rows by swap with supplied vector of rows
ITableDimsTable * ITableDimsTable::create(IRowBaseVec & io_rowVec)
{
    return new TableDimsTable(io_rowVec);
}

// Load table
TableDimsTable::TableDimsTable(IDbExec * i_dbExec, int i_modelId)   
{ 
    const IRowAdapter & adp = TableDimsRowAdapter();
    rowVec = load(
        "SELECT" \
        " M.model_id, M.model_table_id, D.dim_id, D.dim_name, T.model_type_id, D.is_total, D.dim_size" \
        " FROM table_dims D" \
        " INNER JOIN model_table_dic M ON (M.table_hid = D.table_hid)" \
        " INNER JOIN model_type_dic T ON (T.model_id = M.model_id AND T.type_hid = D.type_hid)" +
        ((i_modelId > 0) ? " WHERE M.model_id = " + to_string(i_modelId) : "") +
        " ORDER BY 1, 2, 3", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
TableDimsTable::~TableDimsTable(void) noexcept { }

// Find row by unique key: model id, model table id, dimension id
const TableDimsRow * TableDimsTable::byKey(int i_modelId, int i_tableId, int i_dimId) const
{
    const IRowBaseUptr keyRow( new TableDimsRow(i_modelId, i_tableId, i_dimId) );
    return findKey(keyRow);
}

// get list of rows by model id and table id
vector<TableDimsRow> TableDimsTable::byModelIdTableId(int i_modelId, int i_tableId) const
{
    return findAll(
        [i_modelId, i_tableId](const TableDimsRow & i_row) -> bool {
                return i_row.modelId == i_modelId && i_row.tableId == i_tableId;
            }
    );
}
