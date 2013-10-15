// OpenM++ data library: table_dims table
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // table_dims table implementation
    class TableDimsTable : public ITableDimsTable
    {
    public:
        TableDimsTable(IDbExec * i_dbExec, int i_modelId = 0);
        TableDimsTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~TableDimsTable() throw();

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by primary key: model id, table id, dimension name
        const TableDimsRow * byKey(int i_modelId, int i_tableId, const string & i_name) const;

        // get list of all table rows
        vector<TableDimsRow> rows(void) const { return IMetaTable<TableDimsRow>::rows(rowVec); }

        // get list of rows by model id and table id
        vector<TableDimsRow> byModelIdTableId(int i_modelId, int i_tableId) const;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        TableDimsTable(const TableDimsTable & i_table);             // = delete;
        TableDimsTable & operator=(const TableDimsTable & i_table); // = delete;
    };

    // Columns type for table_dims row
    static const type_info * typeTableDimsRow[] = { 
        &typeid(decltype(TableDimsRow::modelId)), 
        &typeid(decltype(TableDimsRow::tableId)), 
        &typeid(decltype(TableDimsRow::name)), 
        &typeid(decltype(TableDimsRow::pos)), 
        &typeid(decltype(TableDimsRow::typeId)), 
        &typeid(decltype(TableDimsRow::isTotal)),
        &typeid(decltype(TableDimsRow::dimSize))
    };

    // Size (number of columns) for table_dims row
    static const int sizeTableDimsRow = sizeof(typeTableDimsRow) / sizeof(const type_info *);

    // Row adapter to select table_dims rows
    class TableDimsRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new TableDimsRow(); }
        int size(void) const { return sizeTableDimsRow; }
        const type_info ** columnTypes(void) const { return typeTableDimsRow; }

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
                dynamic_cast<TableDimsRow *>(i_row)->name = ((const char *)i_value);
                break;
            case 3:
                dynamic_cast<TableDimsRow *>(i_row)->pos = (*(int *)i_value);
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
ITableDimsTable::~ITableDimsTable(void) throw() { }

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
    rowVec = IMetaTable<TableDimsRow>::load(
        "SELECT" \
        " model_id, table_id, dim_name, dim_pos, mod_type_id, is_total, dim_size" \
        " FROM table_dims" + 
        ((i_modelId > 0) ? " WHERE model_id = " + to_string(i_modelId) : "") +
        " ORDER BY 1, 2, 3", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
TableDimsTable::~TableDimsTable(void) throw() { }

// Find row by primary key: model id, table id, dimension name
const TableDimsRow * TableDimsTable::byKey(int i_modelId, int i_tableId, const string & i_name) const
{
    const IRowBaseUptr keyRow( new TableDimsRow(i_modelId, i_tableId, i_name) );
    return IMetaTable<TableDimsRow>::byKey(keyRow, rowVec);
}

// get list of rows by model id and table id
vector<TableDimsRow> TableDimsTable::byModelIdTableId(int i_modelId, int i_tableId) const
{
    const IRowBaseUptr row( new TableDimsRow(i_modelId, i_tableId, ""));
    return IMetaTable<TableDimsRow>::findAll(row, rowVec, TableDimsRow::modelIdTableIdEqual);
}
