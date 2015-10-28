// OpenM++ data library: profile_lst table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // profile_lst table implementation
    class ProfileLstTable : public IProfileLstTable
    {
    public:
        ProfileLstTable(IDbExec * i_dbExec);
        ProfileLstTable(IRowBaseVec & io_rowVec) {  rowVec.swap(io_rowVec); }
        ~ProfileLstTable() throw();

        // get reference to list of all table rows
        IRowBaseVec & rowsRef(void) { return rowVec; }

        // find row by profile name
        const ProfileLstRow * byKey(const string & i_name) const;

        // get list of all table rows
        vector<ProfileLstRow> rows(void) const { return IMetaTable<ProfileLstRow>::rows(rowVec); }

    private:
        IRowBaseVec rowVec;     // table rows

    private:
        ProfileLstTable(const ProfileLstTable & i_table) = delete;
        ProfileLstTable & operator=(const ProfileLstTable & i_table) = delete;
    };

    // Columns type for profile_lst row
    static const type_info * typeProfileLstRow[] = { 
        &typeid(decltype(ProfileLstRow::name))
    };

    // Size (number of columns) for profile_lst row
    static const int sizeProfileLstRow = sizeof(typeProfileLstRow) / sizeof(const type_info *);

    // Row adapter to select profile_lst rows
    class ProfileLstRowAdapter : public IRowAdapter
    {
    public:
        ProfileLstRowAdapter() { }

        IRowBase * createRow(void) const { return new ProfileLstRow(); }
        int size(void) const { return sizeProfileLstRow; }
        const type_info ** columnTypes(void) const { return typeProfileLstRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<ProfileLstRow *>(i_row)->name = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
IProfileLstTable::~IProfileLstTable(void) throw() { }

// Create new table rows by loading db rows
IProfileLstTable * IProfileLstTable::create(IDbExec * i_dbExec)
{
    return new ProfileLstTable(i_dbExec);
}

// Create new table rows by swap with supplied vector of rows
IProfileLstTable * IProfileLstTable::create(IRowBaseVec & io_rowVec)
{
    return new ProfileLstTable(io_rowVec);
}

// Load table from db
ProfileLstTable::ProfileLstTable(IDbExec * i_dbExec)
{ 
    const IRowAdapter & adp = ProfileLstRowAdapter();
    rowVec = IMetaTable<ProfileLstRow>::load("SELECT profile_name FROM profile_lst ORDER BY 1", i_dbExec, adp);
}

// Table never unloaded
ProfileLstTable::~ProfileLstTable(void) throw() { }

// Find row by primary key: profile name
const ProfileLstRow * ProfileLstTable::byKey(const string & i_name) const
{
    const IRowBaseUptr keyRow( new ProfileLstRow(i_name) );
    return IMetaTable<ProfileLstRow>::byKey(keyRow, rowVec);
}
