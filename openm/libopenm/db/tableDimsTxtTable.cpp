// OpenM++ data library: table_dims_txt table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // table_dims_txt table implementation
    class TableDimsTxtTable : public ITableDimsTxtTable
    {
    public:
        TableDimsTxtTable(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);
        TableDimsTxtTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~TableDimsTxtTable() throw();

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by primary key: model id, table id, dimension id, language id
        const TableDimsTxtRow * byKey(int i_modelId, int i_tableId, int i_dimId, int i_langId) const;

        // get list of all table rows
        vector<TableDimsTxtRow> rows(void) const { return IMetaTable<TableDimsTxtRow>::rows(rowVec); }

        // get list of rows by language
        vector<TableDimsTxtRow> byLang(int i_langId) const;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        TableDimsTxtTable(const TableDimsTxtTable & i_table);               // = delete;
        TableDimsTxtTable & operator=(const TableDimsTxtTable & i_table);   // = delete;
    };

    // Columns type for table_dims_txt row
    static const type_info * typeTableDimsTxtRow[] = { 
        &typeid(decltype(TableDimsTxtRow::modelId)), 
        &typeid(decltype(TableDimsTxtRow::tableId)), 
        &typeid(decltype(TableDimsTxtRow::dimId)), 
        &typeid(decltype(TableDimsTxtRow::langId)), 
        &typeid(decltype(TableDimsTxtRow::descr)), 
        &typeid(decltype(TableDimsTxtRow::note)) 
    };

    // Size (number of columns) for table_dims_txt row
    static const int sizeTableDimsTxtRow = sizeof(typeTableDimsTxtRow) / sizeof(const type_info *);

    // Row adapter to select table_dims_txt rows
    class TableDimsTxtRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new TableDimsTxtRow(); }
        int size(void) const { return sizeTableDimsTxtRow; }
        const type_info ** columnTypes(void) const { return typeTableDimsTxtRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<TableDimsTxtRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<TableDimsTxtRow *>(i_row)->tableId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<TableDimsTxtRow *>(i_row)->dimId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<TableDimsTxtRow *>(i_row)->langId = (*(int *)i_value);
                break;
            case 4:
                dynamic_cast<TableDimsTxtRow *>(i_row)->descr = ((const char *)i_value);
                break;
            case 5:
                dynamic_cast<TableDimsTxtRow *>(i_row)->note = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITableDimsTxtTable::~ITableDimsTxtTable(void) throw() { }

// Create new table rows by loading db rows
ITableDimsTxtTable * ITableDimsTxtTable::create(IDbExec * i_dbExec, int i_modelId, int i_langId)
{
    return new TableDimsTxtTable(i_dbExec, i_modelId, i_langId);
}

// Create new table rows by swap with supplied vector of rows
ITableDimsTxtTable * ITableDimsTxtTable::create(IRowBaseVec & io_rowVec)
{
    return new TableDimsTxtTable(io_rowVec);
}

// Load table
TableDimsTxtTable::TableDimsTxtTable(IDbExec * i_dbExec, int i_modelId, int i_langId)   
{ 
    string sWhere = "";
    if (i_modelId > 0 && i_langId < 0) sWhere = " WHERE model_id = " + to_string(i_modelId);
    if (i_modelId <= 0 && i_langId >= 0) sWhere = " WHERE lang_id = " + to_string(i_langId);
    if (i_modelId > 0 && i_langId >= 0) sWhere = " WHERE model_id = " + to_string(i_modelId) + " AND lang_id = " + to_string(i_langId);

    const IRowAdapter & adp = TableDimsTxtRowAdapter();
    rowVec = IMetaTable<TableDimsTxtRow>::load(
        "SELECT model_id, table_id, dim_id, lang_id, descr, note FROM table_dims_txt" + sWhere + " ORDER BY 1, 2, 3, 4", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
TableDimsTxtTable::~TableDimsTxtTable(void) throw() { }

// Find row by primary key: model id, table id, dimension id, language id
const TableDimsTxtRow * TableDimsTxtTable::byKey(int i_modelId, int i_tableId, int i_dimId, int i_langId) const
{
    const IRowBaseUptr keyRow( new TableDimsTxtRow(i_modelId, i_tableId, i_dimId, i_langId) );
    return IMetaTable<TableDimsTxtRow>::byKey(keyRow, rowVec);
}

// get list of rows by language
vector<TableDimsTxtRow> TableDimsTxtTable::byLang(int i_langId) const
{
    const IRowBaseUptr row( new TableDimsTxtRow(0, 0, 0, i_langId) );
    return IMetaTable<TableDimsTxtRow>::findAll(row, rowVec, TableDimsTxtRow::langEqual);
}
