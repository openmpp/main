// OpenM++ data library: lang_lst table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // lang_lst table implementation
    class LangLstTable : public ILangLstTable
    {
    public:
        LangLstTable(IDbExec * i_dbExec);
        LangLstTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~LangLstTable() noexcept;

        // get const reference to list of all table rows
        const IRowBaseVec& rowsCRef(void) const override { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) override { return rowVec; }

        // find row by language id
        const LangLstRow * byKey(int i_langId) const override;

        // find first row by language code or NULL if not found
        const LangLstRow * byCode(const string & i_code) const override;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        LangLstTable(const LangLstTable & i_table) = delete;
        LangLstTable & operator=(const LangLstTable & i_table) = delete;
    };

    // Columns type for lang_lst row
    static const type_info * typeLangLstRow[] = { 
        &typeid(decltype(LangLstRow::langId)), 
        &typeid(decltype(LangLstRow::code)), 
        &typeid(decltype(LangLstRow::name))
    };

    // Size (number of columns) for lang_lst row
    static const int sizeLangLstRow = sizeof(typeLangLstRow) / sizeof(const type_info *);

    // Row adapter to select lang_lst rows
    class LangLstRowAdapter : public IRowAdapter
    {
    public:
        LangLstRowAdapter() { }

        IRowBase * createRow(void) const { return new LangLstRow(); }
        int size(void) const { return sizeLangLstRow; }
        const type_info * const * columnTypes(void) const { return typeLangLstRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<LangLstRow *>(i_row)->langId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<LangLstRow *>(i_row)->code = ((const char *)i_value);
                break;
            case 2:
                dynamic_cast<LangLstRow *>(i_row)->name = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ILangLstTable::~ILangLstTable(void) noexcept { }

// Create new table rows by loading db rows
ILangLstTable * ILangLstTable::create(IDbExec * i_dbExec)
{
    return new LangLstTable(i_dbExec);
}

// Create new table rows by swap with supplied vector of rows
ILangLstTable * ILangLstTable::create(IRowBaseVec & io_rowVec)
{
    return new LangLstTable(io_rowVec);
}

// Load table from db
LangLstTable::LangLstTable(IDbExec * i_dbExec)
{ 
    string sql = "SELECT lang_id, lang_code, lang_name FROM lang_lst ORDER BY 1";

    const IRowAdapter & adp = LangLstRowAdapter();
    rowVec = load(sql, i_dbExec, adp);
}

// Table never unloaded
LangLstTable::~LangLstTable(void) noexcept { }

// Find row by primary key: language id
const LangLstRow * LangLstTable::byKey(int i_langId) const
{
    const IRowBaseUptr keyRow( new LangLstRow(i_langId) );
    return findKey(keyRow);
}

// find first row by language code or NULL if not found
const LangLstRow * LangLstTable::byCode(const string & i_code) const
{
    return findFirst(
        [i_code](const LangLstRow & i_row) -> bool { return i_row.code == i_code; }
    );
}
