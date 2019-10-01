// OpenM++ data library: table_dic_txt join to model_table_dic table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // table_dic_txt join to model_table_dic table implementation
    class TableDicTxtTable : public ITableDicTxtTable
    {
    public:
        TableDicTxtTable(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);
        TableDicTxtTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~TableDicTxtTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec& rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) override { return rowVec; }

        // find row by unique key: model id, model table id, language id
        const TableDicTxtRow * byKey(int i_modelId, int i_tableId, int i_langId) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        TableDicTxtTable(const TableDicTxtTable & i_table) = delete;
        TableDicTxtTable & operator=(const TableDicTxtTable & i_table) = delete;
    };

    // Columns type for table_dic_txt join to model_table_dic row
    static const type_info * typeTableDicTxtRow[] = { 
        &typeid(decltype(TableDicTxtRow::modelId)), 
        &typeid(decltype(TableDicTxtRow::tableId)), 
        &typeid(decltype(TableDicTxtRow::langId)), 
        &typeid(decltype(TableDicTxtRow::descr)), 
        &typeid(decltype(TableDicTxtRow::note)), 
        &typeid(decltype(TableDicTxtRow::exprDescr)), 
        &typeid(decltype(TableDicTxtRow::exprNote))
    };

    // Size (number of columns) for table_dic_txt join to model_table_dic row
    static const int sizeTableDicTxtRow = sizeof(typeTableDicTxtRow) / sizeof(const type_info *);

    // Row adapter to select table_dic_txt join to model_table_dic rows
    class TableDicTxtRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new TableDicTxtRow(); }
        int size(void) const { return sizeTableDicTxtRow; }
        const type_info * const * columnTypes(void) const { return typeTableDicTxtRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<TableDicTxtRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<TableDicTxtRow *>(i_row)->tableId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<TableDicTxtRow *>(i_row)->langId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<TableDicTxtRow *>(i_row)->descr = ((const char *)i_value);
                break;
            case 4:
                dynamic_cast<TableDicTxtRow *>(i_row)->note = ((const char *)i_value);
                break;
            case 5:
                dynamic_cast<TableDicTxtRow *>(i_row)->exprDescr = ((const char *)i_value);
                break;
            case 6:
                dynamic_cast<TableDicTxtRow *>(i_row)->exprNote = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITableDicTxtTable::~ITableDicTxtTable(void) noexcept { }

// Create new table rows by loading db rows
ITableDicTxtTable * ITableDicTxtTable::create(IDbExec * i_dbExec, int i_modelId, int i_langId)
{
    return new TableDicTxtTable(i_dbExec, i_modelId, i_langId);
}

// Create new table rows by swap with supplied vector of rows
ITableDicTxtTable * ITableDicTxtTable::create(IRowBaseVec & io_rowVec)
{
    return new TableDicTxtTable(io_rowVec);
}

// Load table
TableDicTxtTable::TableDicTxtTable(IDbExec * i_dbExec, int i_modelId, int i_langId)
{ 
    string sWhere = "";
    if (i_modelId > 0 && i_langId < 0) sWhere = " WHERE M.model_id = " + to_string(i_modelId);
    if (i_modelId <= 0 && i_langId >= 0) sWhere = " WHERE T.lang_id = " + to_string(i_langId);
    if (i_modelId > 0 && i_langId >= 0) sWhere = " WHERE M.model_id = " + to_string(i_modelId) + " AND T.lang_id = " + to_string(i_langId);

    const IRowAdapter & adp = TableDicTxtRowAdapter();
    rowVec = load(
        "SELECT M.model_id, M.model_table_id, T.lang_id, T.descr, T.note, T.expr_descr, T.expr_note" \
        " FROM table_dic_txt T" \
        " INNER JOIN model_table_dic M ON (M.table_hid = T.table_hid)" +
        sWhere + 
        " ORDER BY 1, 2, 3", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
TableDicTxtTable::~TableDicTxtTable(void) noexcept { }

// Find row by unique key: model id, model table id, language id
const TableDicTxtRow * TableDicTxtTable::byKey(int i_modelId, int i_tableId, int i_langId) const
{
    const IRowBaseUptr keyRow( new TableDicTxtRow(i_modelId, i_tableId, i_langId) );
    return findKey(keyRow);
}

