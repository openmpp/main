// OpenM++ data library: profile_option table
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // profile_option table implementation
    class ProfileOptionTable : public IProfileOptionTable
    {
    public:
        ProfileOptionTable(IDbExec * i_dbExec, const string & i_name = "");
        ProfileOptionTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~ProfileOptionTable() throw();

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by primary key: profile name, option key
        const ProfileOptionRow * byKey(const string & i_name, const string & i_key) const;

        // get list of all table rows
        vector<ProfileOptionRow> rows(void) const { return IMetaTable<ProfileOptionRow>::rows(rowVec); }

        // get list of rows by profile name
        vector<ProfileOptionRow> byName(const string & i_name) const;

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        ProfileOptionTable(const ProfileOptionTable & i_table);               // = delete;
        ProfileOptionTable & operator=(const ProfileOptionTable & i_table);   // = delete;
    };

    // Columns type for profile_option row
    static const type_info * typeProfileOptionRow[] = { 
        &typeid(decltype(ProfileOptionRow::name)), 
        &typeid(decltype(ProfileOptionRow::key)), 
        &typeid(decltype(ProfileOptionRow::value))
    };

    // Size (number of columns) for profile_option row
    static const int sizeProfileOptionRow = sizeof(typeProfileOptionRow) / sizeof(const type_info *);

    // Row adapter to select profile_option rows
    class ProfileOptionRowAdapter : public IRowAdapter
    {
    public:
        IRowBase * createRow(void) const { return new ProfileOptionRow(); }
        int size(void) const { return sizeProfileOptionRow; }
        const type_info ** columnTypes(void) const { return typeProfileOptionRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<ProfileOptionRow *>(i_row)->name = ((const char *)i_value);
                break;
            case 1:
                dynamic_cast<ProfileOptionRow *>(i_row)->key = ((const char *)i_value);
                break;
            case 2:
                dynamic_cast<ProfileOptionRow *>(i_row)->value = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IProfileOptionTable::~IProfileOptionTable(void) throw() { }

// Create new table rows by loading db rows
IProfileOptionTable * IProfileOptionTable::create(IDbExec * i_dbExec, const string & i_name)
{
    return new ProfileOptionTable(i_dbExec, i_name);
}

// Create new table rows by swap with supplied vector of rows
IProfileOptionTable * IProfileOptionTable::create(IRowBaseVec & io_rowVec)
{
    return new ProfileOptionTable(io_rowVec);
}

// Load table
ProfileOptionTable::ProfileOptionTable(IDbExec * i_dbExec, const string & i_name)
{ 
    const IRowAdapter & adp = ProfileOptionRowAdapter();
    rowVec = IMetaTable<ProfileOptionRow>::load(
        "SELECT profile_name, option_key, option_value" \
        " FROM profile_option" +
        ((i_name != "") ? " WHERE profile_name = " + toQuoted(i_name) : "") +
        " ORDER BY 1, 2",
        i_dbExec, 
        adp
        );
}

// Table never unloaded
ProfileOptionTable::~ProfileOptionTable(void) throw() { }

// Find row by primary key: profile name, option key
const ProfileOptionRow * ProfileOptionTable::byKey(const string & i_name, const string & i_key) const
{
    const IRowBaseUptr keyRow( new ProfileOptionRow(i_name, i_key) );
    return IMetaTable<ProfileOptionRow>::byKey(keyRow, rowVec);
}

// get list of rows by profile name
vector<ProfileOptionRow> ProfileOptionTable::byName(const string & i_name) const
{
    const IRowBaseUptr row( new ProfileOptionRow(i_name, "") );
    return IMetaTable<ProfileOptionRow>::findAll(row, rowVec, ProfileOptionRow::nameEqual);
}
