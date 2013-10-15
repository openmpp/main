// OpenM++ data library: table_unit table
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // table_unit table implementation
    class TableUnitTable : public ITableUnitTable
    {
    public:
        TableUnitTable(IDbExec * i_dbExec, int i_modelId = 0);
        TableUnitTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~TableUnitTable() throw();

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by primary key: model id, table id, unit id
        const TableUnitRow * byKey(int i_modelId, int i_tableId, int i_unitId) const;

        // get list of all table rows
        vector<TableUnitRow> rows(void) const { return IMetaTable<TableUnitRow>::rows(rowVec); }

        // get list of rows by model id and table id
        vector<TableUnitRow> byModelIdTableId(int i_modelId, int i_tableId) const;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        TableUnitTable(const TableUnitTable & i_table);             // = delete;
        TableUnitTable & operator=(const TableUnitTable & i_table); // = delete;
    };

    // Columns type for table_unit row
    static const type_info * typeTableUnitRow[] = { 
        &typeid(decltype(TableUnitRow::modelId)), 
        &typeid(decltype(TableUnitRow::tableId)), 
        &typeid(decltype(TableUnitRow::unitId)), 
        &typeid(decltype(TableUnitRow::name)), 
        &typeid(decltype(TableUnitRow::decimals)),
        &typeid(decltype(TableUnitRow::src)), 
        &typeid(decltype(TableUnitRow::expr)) 
    };

    // Size (number of columns) for table_unit row
    static const int sizeTableUnitRow = sizeof(typeTableUnitRow) / sizeof(const type_info *);

    // Row adapter to select table_unit rows
    class TableUnitRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new TableUnitRow(); }
        int size(void) const { return sizeTableUnitRow; }
        const type_info ** columnTypes(void) const { return typeTableUnitRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<TableUnitRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<TableUnitRow *>(i_row)->tableId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<TableUnitRow *>(i_row)->unitId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<TableUnitRow *>(i_row)->name = ((const char *)i_value);
                break;
            case 4:
                dynamic_cast<TableUnitRow *>(i_row)->decimals = (*(int *)i_value);
                break;
            case 5:
                dynamic_cast<TableUnitRow *>(i_row)->src = ((const char *)i_value);
                break;
            case 6:
                dynamic_cast<TableUnitRow *>(i_row)->expr = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITableUnitTable::~ITableUnitTable(void) throw() { }

// Create new table rows by loading db rows
ITableUnitTable * ITableUnitTable::create(IDbExec * i_dbExec, int i_modelId)
{
    return new TableUnitTable(i_dbExec, i_modelId);
}

// Create new table rows by swap with supplied vector of rows
ITableUnitTable * ITableUnitTable::create(IRowBaseVec & io_rowVec)
{
    return new TableUnitTable(io_rowVec);
}

// Load table
TableUnitTable::TableUnitTable(IDbExec * i_dbExec, int i_modelId)
{ 
    const IRowAdapter & adp = TableUnitRowAdapter();
    rowVec = IMetaTable<TableUnitRow>::load(
        "SELECT" \
        " model_id, table_id, unit_id, unit_name, unit_decimals, unit_src, unit_expr" \
        " FROM table_unit" + 
        ((i_modelId > 0) ? " WHERE model_id = " + to_string(i_modelId) : "") +
        " ORDER BY 1, 2, 3", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
TableUnitTable::~TableUnitTable(void) throw() { }

// Find row by primary key: model id, table id, unit id
const TableUnitRow * TableUnitTable::byKey(int i_modelId, int i_tableId, int i_unitId) const
{
    const IRowBaseUptr keyRow( new TableUnitRow(i_modelId, i_tableId, i_unitId) );
    return IMetaTable<TableUnitRow>::byKey(keyRow, rowVec);
}

// get list of rows by model id and table id
vector<TableUnitRow> TableUnitTable::byModelIdTableId(int i_modelId, int i_tableId) const
{
    const IRowBaseUptr row( new TableUnitRow(i_modelId, i_tableId, 0));
    return IMetaTable<TableUnitRow>::findAll(row, rowVec, TableUnitRow::modelIdTableIdEqual);
}
