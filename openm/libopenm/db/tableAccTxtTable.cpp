// OpenM++ data library: table_acc_txt table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // table_acc_txt table implementation
    class TableAccTxtTable : public ITableAccTxtTable
    {
    public:
        TableAccTxtTable(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);
        TableAccTxtTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~TableAccTxtTable() throw();

        // get const reference to list of all table rows
        const IRowBaseVec & rowsCRef(void) const { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by primary key: model id, table id, accumulator id, language id
        const TableAccTxtRow * byKey(int i_modelId, int i_tableId, int i_accId, int i_langId) const;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        TableAccTxtTable(const TableAccTxtTable & i_table) = delete;
        TableAccTxtTable & operator=(const TableAccTxtTable & i_table) = delete;
    };

    // Columns type for table_acc_txt row
    static const type_info * typeTableAccTxtRow[] = { 
        &typeid(decltype(TableAccTxtRow::modelId)), 
        &typeid(decltype(TableAccTxtRow::tableId)), 
        &typeid(decltype(TableAccTxtRow::accId)), 
        &typeid(decltype(TableAccTxtRow::langId)), 
        &typeid(decltype(TableAccTxtRow::descr)), 
        &typeid(decltype(TableAccTxtRow::note)) 
    };

    // Size (number of columns) for table_acc_txt row
    static const int sizeTableAccTxtRow = sizeof(typeTableAccTxtRow) / sizeof(const type_info *);

    // Row adapter to select table_acc_txt rows
    class TableAccTxtRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new TableAccTxtRow(); }
        int size(void) const { return sizeTableAccTxtRow; }
        const type_info ** columnTypes(void) const { return typeTableAccTxtRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<TableAccTxtRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<TableAccTxtRow *>(i_row)->tableId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<TableAccTxtRow *>(i_row)->accId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<TableAccTxtRow *>(i_row)->langId = (*(int *)i_value);
                break;
            case 4:
                dynamic_cast<TableAccTxtRow *>(i_row)->descr = ((const char *)i_value);
                break;
            case 5:
                dynamic_cast<TableAccTxtRow *>(i_row)->note = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITableAccTxtTable::~ITableAccTxtTable(void) throw() { }

// Create new table rows by loading db rows
ITableAccTxtTable * ITableAccTxtTable::create(IDbExec * i_dbExec, int i_modelId, int i_langId)
{
    return new TableAccTxtTable(i_dbExec, i_modelId, i_langId);
}

// Create new table rows by swap with supplied vector of rows
ITableAccTxtTable * ITableAccTxtTable::create(IRowBaseVec & io_rowVec)
{
    return new TableAccTxtTable(io_rowVec);
}

// Load table
TableAccTxtTable::TableAccTxtTable(IDbExec * i_dbExec, int i_modelId, int i_langId)
{ 
    string sWhere = "";
    if (i_modelId > 0 && i_langId < 0) sWhere = " WHERE model_id = " + to_string(i_modelId);
    if (i_modelId <= 0 && i_langId >= 0) sWhere = " WHERE lang_id = " + to_string(i_langId);
    if (i_modelId > 0 && i_langId >= 0) sWhere = " WHERE model_id = " + to_string(i_modelId) + " AND lang_id = " + to_string(i_langId);

    const IRowAdapter & adp = TableAccTxtRowAdapter();
    rowVec = load(
        "SELECT model_id, table_id, acc_id, lang_id, descr, note FROM table_acc_txt" +  sWhere + " ORDER BY 1, 2, 3, 4", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
TableAccTxtTable::~TableAccTxtTable(void) throw() { }

// Find row by primary key: model id, table id, accumulator name, language id
const TableAccTxtRow * TableAccTxtTable::byKey(int i_modelId, int i_tableId, int i_accId, int i_langId) const
{
    const IRowBaseUptr keyRow( new TableAccTxtRow(i_modelId, i_tableId, i_accId, i_langId) );
    return findKey(keyRow);
}
