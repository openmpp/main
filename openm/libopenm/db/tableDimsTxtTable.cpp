// OpenM++ data library: table_dims_txt join to model_table_dic table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // table_dims_txt join to model_table_dic table implementation
    class TableDimsTxtTable : public ITableDimsTxtTable
    {
    public:
        TableDimsTxtTable(IDbExec * i_dbExec, int i_modelId = 0, int i_langId = -1);
        TableDimsTxtTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~TableDimsTxtTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec& rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by unique key: model id, model table id, dimension id, language id
        const TableDimsTxtRow * byKey(int i_modelId, int i_tableId, int i_dimId, int i_langId) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        TableDimsTxtTable(const TableDimsTxtTable & i_table) = delete;
        TableDimsTxtTable & operator=(const TableDimsTxtTable & i_table) = delete;
    };

    // Columns type for table_dims_txt join to model_table_dic row
    static const type_info * typeTableDimsTxtRow[] = { 
        &typeid(decltype(TableDimsTxtRow::modelId)), 
        &typeid(decltype(TableDimsTxtRow::tableId)), 
        &typeid(decltype(TableDimsTxtRow::dimId)), 
        &typeid(decltype(TableDimsTxtRow::langId)), 
        &typeid(decltype(TableDimsTxtRow::descr)), 
        &typeid(decltype(TableDimsTxtRow::note)) 
    };

    // Size (number of columns) for table_dims_txt join to model_table_dic row
    static const int sizeTableDimsTxtRow = sizeof(typeTableDimsTxtRow) / sizeof(const type_info *);

    // Row adapter to select table_dims_txt join to model_table_dic rows
    class TableDimsTxtRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new TableDimsTxtRow(); }
        int size(void) const { return sizeTableDimsTxtRow; }
        const type_info * const * columnTypes(void) const { return typeTableDimsTxtRow; }

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
ITableDimsTxtTable::~ITableDimsTxtTable(void) noexcept { }

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
    if (i_modelId > 0 && i_langId < 0) sWhere = " WHERE M.model_id = " + to_string(i_modelId);
    if (i_modelId <= 0 && i_langId >= 0) sWhere = " WHERE D.lang_id = " + to_string(i_langId);
    if (i_modelId > 0 && i_langId >= 0) sWhere = " WHERE M.model_id = " + to_string(i_modelId) + " AND D.lang_id = " + to_string(i_langId);

    const IRowAdapter & adp = TableDimsTxtRowAdapter();
    rowVec = load(
        "SELECT M.model_id, M.model_table_id, D.dim_id, D.lang_id, D.descr, D.note" \
        " FROM table_dims_txt D" \
        " INNER JOIN model_table_dic M ON (M.table_hid = D.table_hid)" +
        sWhere + 
        " ORDER BY 1, 2, 3, 4", 
        i_dbExec,
        adp
        );
}

// Table never unloaded
TableDimsTxtTable::~TableDimsTxtTable(void) noexcept { }

// Find row by unique key: model id, model table id, dimension id, language id
const TableDimsTxtRow * TableDimsTxtTable::byKey(int i_modelId, int i_tableId, int i_dimId, int i_langId) const
{
    const IRowBaseUptr keyRow( new TableDimsTxtRow(i_modelId, i_tableId, i_dimId, i_langId) );
    return findKey(keyRow);
}

