// OpenM++ data library: lang_word table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // lang_word table implementation
    class LangWordTable : public ILangWordTable
    {
    public:
        LangWordTable(IDbExec * i_dbExec);
        LangWordTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~LangWordTable() throw();

        // get const reference to list of all table rows
        const IRowBaseVec & rowsCRef(void) const { return rowVec; }

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by language id and word code
        const LangWordRow * byKey(int i_langId, const string & i_code) const;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        LangWordTable(const LangWordTable & i_table) = delete;
        LangWordTable & operator=(const LangWordTable & i_table) = delete;
    };

    // Columns type for lang_word row
    static const type_info * typeLangWordRow[] = { 
        &typeid(decltype(LangWordRow::langId)), 
        &typeid(decltype(LangWordRow::code)), 
        &typeid(decltype(LangWordRow::value))
    };

    // Size (number of columns) for lang_word row
    static const int sizeLangWordRow = sizeof(typeLangWordRow) / sizeof(const type_info *);

    // Row adapter to select lang_word rows
    class LangWordRowAdapter : public IRowAdapter
    {
    public:
        LangWordRowAdapter() { }

        IRowBase * createRow(void) const { return new LangWordRow(); }
        int size(void) const { return sizeLangWordRow; }
        const type_info * const * columnTypes(void) const { return typeLangWordRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<LangWordRow *>(i_row)->langId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<LangWordRow *>(i_row)->code = ((const char *)i_value);
                break;
            case 2:
                dynamic_cast<LangWordRow *>(i_row)->value = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ILangWordTable::~ILangWordTable(void) throw() { }

// Create new table rows by loading db rows
ILangWordTable * ILangWordTable::create(IDbExec * i_dbExec)
{
    return new LangWordTable(i_dbExec);
}

// Create new table rows by swap with supplied vector of rows
ILangWordTable * ILangWordTable::create(IRowBaseVec & io_rowVec)
{
    return new LangWordTable(io_rowVec);
}

// Load table from db
LangWordTable::LangWordTable(IDbExec * i_dbExec)
{ 
    string sql = "SELECT lang_id, word_code, word_value FROM lang_word ORDER BY 1, 2";

    const IRowAdapter & adp = LangWordRowAdapter();
    rowVec = load(sql, i_dbExec, adp);
}

// Table never unloaded
LangWordTable::~LangWordTable(void) throw() { }

// Find row by primary key: language id and word code
const LangWordRow * LangWordTable::byKey(int i_langId, const string & i_code) const
{
    const IRowBaseUptr keyRow( new LangWordRow(i_langId, i_code) );
    return findKey(keyRow);
}
