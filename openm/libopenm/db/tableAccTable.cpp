// OpenM++ data library: table_acc join to model_table_dic table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // table_acc join to model_table_dic table implementation
    class TableAccTable : public ITableAccTable
    {
    public:
        TableAccTable(IDbExec * i_dbExec, int i_modelId, bool i_isIncludeDerived);
        TableAccTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~TableAccTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec& rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by unique key: model id, model table id, accumulator id
        const TableAccRow * byKey(int i_modelId, int i_tableId, int i_accId) const override;

        // get list of rows by model id
        vector<TableAccRow> byModelId(int i_modelId) const override;

        // get list of rows by model id and table id
        vector<TableAccRow> byModelIdTableId(int i_modelId, int i_tableId) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        TableAccTable(const TableAccTable & i_table) = delete;
        TableAccTable & operator=(const TableAccTable & i_table) = delete;
    };

    // Columns type for table_acc join to model_table_dic row
    static const type_info * typeTableAccRow[] = { 
        &typeid(decltype(TableAccRow::modelId)), 
        &typeid(decltype(TableAccRow::tableId)), 
        &typeid(decltype(TableAccRow::accId)), 
        &typeid(decltype(TableAccRow::name)), 
        &typeid(decltype(TableAccRow::isDerived)), 
        &typeid(decltype(TableAccRow::accSrc)),
        &typeid(decltype(TableAccRow::accSql))
    };

    // Size (number of columns) for table_acc join to model_table_dic row
    static const int sizeTableAccRow = sizeof(typeTableAccRow) / sizeof(const type_info *);

    // Row adapter to select table_acc join to model_table_dic rows
    class TableAccRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new TableAccRow(); }
        int size(void) const { return sizeTableAccRow; }
        const type_info * const * columnTypes(void) const { return typeTableAccRow; }

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
                dynamic_cast<TableAccRow *>(i_row)->isDerived = (*(bool *)i_value);
                break;
            case 5:
                dynamic_cast<TableAccRow *>(i_row)->accSrc = ((const char *)i_value);
                break;
            case 6:
                dynamic_cast<TableAccRow *>(i_row)->accSql = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITableAccTable::~ITableAccTable(void) noexcept { }

// Create new table rows by loading db rows
ITableAccTable * ITableAccTable::create(IDbExec * i_dbExec, int i_modelId, bool i_isIncludeDerived)
{
    return new TableAccTable(i_dbExec, i_modelId, i_isIncludeDerived);
}

// Create new table rows by swap with supplied vector of rows
ITableAccTable * ITableAccTable::create(IRowBaseVec & io_rowVec)
{
    return new TableAccTable(io_rowVec);
}

// Load table
TableAccTable::TableAccTable(IDbExec * i_dbExec, int i_modelId, bool i_isIncludeDerived)
{ 
    const IRowAdapter & adp = TableAccRowAdapter();
    rowVec = load(
        "SELECT M.model_id, M.model_table_id, D.acc_id, D.acc_name, D.is_derived, D.acc_src, D.acc_sql" \
        " FROM table_acc D" \
        " INNER JOIN model_table_dic M ON (M.table_hid = D.table_hid)" +
        ((i_modelId > 0) ? " WHERE M.model_id = " + to_string(i_modelId) : "") +
        ((!i_isIncludeDerived) ? " AND D.is_derived = 0" : "") +
        " ORDER BY 1, 2, 3", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
TableAccTable::~TableAccTable(void) noexcept { }

// Find row by unique key: model id, model table id, accumulator id
const TableAccRow * TableAccTable::byKey(int i_modelId, int i_tableId, int i_accId) const
{
    const IRowBaseUptr keyRow( new TableAccRow(i_modelId, i_tableId, i_accId) );
    return findKey(keyRow);
}

// get list of rows by model id
vector<TableAccRow> TableAccTable::byModelId(int i_modelId) const
{
    return findAll(
        [i_modelId](const TableAccRow & i_row) -> bool { return i_row.modelId == i_modelId; }
    );
}

// get list of rows by model id and table id
vector<TableAccRow> TableAccTable::byModelIdTableId(int i_modelId, int i_tableId) const
{
    return findAll(
        [i_modelId, i_tableId](const TableAccRow & i_row) -> bool {
                return i_row.modelId == i_modelId && i_row.tableId == i_tableId;
            }
    );
}
