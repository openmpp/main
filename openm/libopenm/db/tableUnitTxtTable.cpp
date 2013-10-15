// OpenM++ data library: table_unit_txt table
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // table_unit_txt table implementation
    class TableUnitTxtTable : public ITableUnitTxtTable
    {
    public:
        TableUnitTxtTable(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);
        TableUnitTxtTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~TableUnitTxtTable() throw();

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by primary key: model id, table id, unit id, language id
        const TableUnitTxtRow * byKey(int i_modelId, int i_tableId, int i_unitId, int i_langId) const;

        // get list of all table rows
        vector<TableUnitTxtRow> rows(void) const { return IMetaTable<TableUnitTxtRow>::rows(rowVec); }

        // get list of rows by language
        vector<TableUnitTxtRow> byLang(int i_langId) const;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        TableUnitTxtTable(const TableUnitTxtTable & i_table);               // = delete;
        TableUnitTxtTable & operator=(const TableUnitTxtTable & i_table);   // = delete;
    };

    // Columns type for table_unit_txt row
    static const type_info * typeTableUnitTxtRow[] = { 
        &typeid(decltype(TableUnitTxtRow::modelId)), 
        &typeid(decltype(TableUnitTxtRow::tableId)), 
        &typeid(decltype(TableUnitTxtRow::unitId)), 
        &typeid(decltype(TableUnitTxtRow::langId)), 
        &typeid(decltype(TableUnitTxtRow::descr)), 
        &typeid(decltype(TableUnitTxtRow::note)) 
    };

    // Size (number of columns) for table_unit_txt row
    static const int sizeTableUnitTxtRow = sizeof(typeTableUnitTxtRow) / sizeof(const type_info *);

    // Row adapter to select table_unit_txt rows
    class TableUnitTxtRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new TableUnitTxtRow(); }
        int size(void) const { return sizeTableUnitTxtRow; }
        const type_info ** columnTypes(void) const { return typeTableUnitTxtRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<TableUnitTxtRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<TableUnitTxtRow *>(i_row)->tableId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<TableUnitTxtRow *>(i_row)->unitId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<TableUnitTxtRow *>(i_row)->langId = (*(int *)i_value);
                break;
            case 4:
                dynamic_cast<TableUnitTxtRow *>(i_row)->descr = ((const char *)i_value);
                break;
            case 5:
                dynamic_cast<TableUnitTxtRow *>(i_row)->note = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITableUnitTxtTable::~ITableUnitTxtTable(void) throw() { }

// Create new table rows by loading db rows
ITableUnitTxtTable * ITableUnitTxtTable::create(IDbExec * i_dbExec, int i_modelId, int i_langId)
{
    return new TableUnitTxtTable(i_dbExec, i_modelId, i_langId);
}

// Create new table rows by swap with supplied vector of rows
ITableUnitTxtTable * ITableUnitTxtTable::create(IRowBaseVec & io_rowVec)
{
    return new TableUnitTxtTable(io_rowVec);
}

// Load table
TableUnitTxtTable::TableUnitTxtTable(IDbExec * i_dbExec, int i_modelId, int i_langId)
{ 
    string sWhere = "";
    if (i_modelId > 0 && i_langId < 0) sWhere = " WHERE model_id = " + to_string(i_modelId);
    if (i_modelId <= 0 && i_langId >= 0) sWhere = " WHERE lang_id = " + to_string(i_langId);
    if (i_modelId > 0 && i_langId >= 0) sWhere = " WHERE model_id = " + to_string(i_modelId) + " AND lang_id = " + to_string(i_langId);

    const IRowAdapter & adp = TableUnitTxtRowAdapter();
    rowVec = IMetaTable<TableUnitTxtRow>::load(
        "SELECT model_id, table_id, unit_id, lang_id, descr, note FROM table_unit_txt" +  sWhere + " ORDER BY 1, 2, 3, 4", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
TableUnitTxtTable::~TableUnitTxtTable(void) throw() { }

// Find row by primary key: model id, table id, unit id, language id
const TableUnitTxtRow * TableUnitTxtTable::byKey(int i_modelId, int i_tableId, int i_unitId, int i_langId) const
{
    const IRowBaseUptr keyRow( new TableUnitTxtRow(i_modelId, i_tableId, i_unitId, i_langId) );
    return IMetaTable<TableUnitTxtRow>::byKey(keyRow, rowVec);
}

// get list of rows by language
vector<TableUnitTxtRow> TableUnitTxtTable::byLang(int i_langId) const
{
    const IRowBaseUptr row( new TableUnitTxtRow(0, 0, 0, i_langId) );
    return IMetaTable<TableUnitTxtRow>::findAll(row, rowVec, TableUnitTxtRow::langEqual);
}
