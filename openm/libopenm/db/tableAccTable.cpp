// OpenM++ data library: table_acc table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // table_acc table implementation
    class TableAccTable : public ITableAccTable
    {
    public:
        TableAccTable(IDbExec * i_dbExec, int i_modelId = 0);
        TableAccTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~TableAccTable() throw();

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by primary key: model id, table id, accumulator id
        const TableAccRow * byKey(int i_modelId, int i_tableId, int i_accId) const;

        // get list of all table rows
        vector<TableAccRow> rows(void) const { return IMetaTable<TableAccRow>::rows(rowVec); }

        // get list of rows by model id
        vector<TableAccRow> byModelId(int i_modelId) const;

        // get list of rows by model id and table id
        vector<TableAccRow> byModelIdTableId(int i_modelId, int i_tableId) const;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        TableAccTable(const TableAccTable & i_table);             // = delete;
        TableAccTable & operator=(const TableAccTable & i_table); // = delete;
    };

    // Columns type for table_acc row
    static const type_info * typeTableAccRow[] = { 
        &typeid(decltype(TableAccRow::modelId)), 
        &typeid(decltype(TableAccRow::tableId)), 
        &typeid(decltype(TableAccRow::accId)), 
        &typeid(decltype(TableAccRow::name)), 
        &typeid(decltype(TableAccRow::expr))
    };

    // Size (number of columns) for table_acc row
    static const int sizeTableAccRow = sizeof(typeTableAccRow) / sizeof(const type_info *);

    // Row adapter to select table_acc rows
    class TableAccRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new TableAccRow(); }
        int size(void) const { return sizeTableAccRow; }
        const type_info ** columnTypes(void) const { return typeTableAccRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<TableAccRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<TableAccRow *>(i_row)->tableId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<TableAccRow *>(i_row)->accId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<TableAccRow *>(i_row)->name = ((const char *)i_value);
                break;
            case 4:
                dynamic_cast<TableAccRow *>(i_row)->expr = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITableAccTable::~ITableAccTable(void) throw() { }

// Create new table rows by loading db rows
ITableAccTable * ITableAccTable::create(IDbExec * i_dbExec, int i_modelId)
{
    return new TableAccTable(i_dbExec, i_modelId);
}

// Create new table rows by swap with supplied vector of rows
ITableAccTable * ITableAccTable::create(IRowBaseVec & io_rowVec)
{
    return new TableAccTable(io_rowVec);
}

// Load table
TableAccTable::TableAccTable(IDbExec * i_dbExec, int i_modelId)
{ 
    const IRowAdapter & adp = TableAccRowAdapter();
    rowVec = IMetaTable<TableAccRow>::load(
        "SELECT model_id, table_id, acc_id, acc_name, acc_expr FROM table_acc" + 
        ((i_modelId > 0) ? " WHERE model_id = " + to_string(i_modelId) : "") +
        " ORDER BY 1, 2, 3", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
TableAccTable::~TableAccTable(void) throw() { }

// Find row by primary key: model id, table id, accumulator id
const TableAccRow * TableAccTable::byKey(int i_modelId, int i_tableId, int i_accId) const
{
    const IRowBaseUptr keyRow( new TableAccRow(i_modelId, i_tableId, i_accId) );
    return IMetaTable<TableAccRow>::byKey(keyRow, rowVec);
}

// get list of rows by model id
vector<TableAccRow> TableAccTable::byModelId(int i_modelId) const
{
    const IRowBaseUptr row( new TableAccRow(i_modelId, 0, 0));
    return IMetaTable<TableAccRow>::findAll(row, rowVec, TableAccRow::modelIdEqual);
}

// get list of rows by model id and table id
vector<TableAccRow> TableAccTable::byModelIdTableId(int i_modelId, int i_tableId) const
{
    const IRowBaseUptr row( new TableAccRow(i_modelId, i_tableId, 0));
    return IMetaTable<TableAccRow>::findAll(row, rowVec, TableAccRow::modelIdTableIdEqual);
}
