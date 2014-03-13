/**
 * @file
 * OpenM++ common file utilities.
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef OM_FILE_H
#define OM_FILE_H

#include "libopenm/common/omHelper.h"

using namespace std;

namespace openm
{
    /** return true if file exists  */
    extern bool isFileExists(const char * i_filePath);

    /** make path from current working directory, executable name and specified extension. */
    extern string makeDefaultPath(const char * i_exePath, const char * i_extension);

    /** convert null-terminated bytes from current user (or specified) code page to UTF-8 string. */
    extern string toUtf8(const char * i_byteArr, const char * i_codePageName = NULL);

    /** convert bytes from current user (or specified) code page to UTF-8 string. */
    extern string toUtf8(size_t i_size, const char * i_byteArr, const char * i_codePageName = NULL);

    /** read file and return content as UTF-8 as string */
    extern string fileToUtf8(const char * i_filePath, const char * i_codePageName = NULL);

    /** Ini-file entry: setcion, key and value */
    struct IniEntry
    {
        /** section: in lower case, without [], space trimed */
        string section;

        /** key: in lower case, without =, space trimed */
        string key;

        /** value: space trimed and unquoted ("quotes" or 'apostrophes' removed) */
        string val;

        /** section as it was in source ini-file */
        string sectionSource;

        /** key as it was in source ini-file */
        string keySource;

        /**
         * initialize ini-file entry.
         *
         * @param[in]   i_sectionSource ini-file section name.
         * @param[in]   i_keySource     ini-file key.
         * @param[in]   i_value         ini-file entry value.
         */
        IniEntry(const string & i_sectionSource, const string & i_keySource, const string & i_value);

        /** ini-file entry section and key case neutral equal comparison. */
        bool equalTo(const IniEntry & i_other) const;

        /** ini-file entry section.key case neutral equal comparison. */
        bool bySectionKey(const char * i_sectionKey) const;

    private:
        // set section name by sectionSource: remove [brackets], convert to lower case and trim
        void setSection(void);

        // set key name by keySource: remove = if present, convert to lower and trim
        void setKey(void);

        // set value: space trimed, comment removed and unquoted ("quotes" or 'apostrophes' removed)
        void setValue(const string & i_value);
    };

    /** ini-file reader: load all entries in memory and provide search methods. */
    class IniFileReader
    {
        /** ini-file entries: vector of (section, key, value). */
        typedef vector<IniEntry> IniEntryVec;

    public:
        /** load all ini-file entries in memory. */
        IniFileReader(const char * i_filePath);

        /** return true if ini-file loaded correctly or false on error */
        bool isLoaded(void) const throw() { return is_loaded; }

        /** return true if section and key exist in ini-file. */
        bool isExist(const char * i_section, const char * i_key) const throw();

        /** return true if section.key exist in ini-file. */
        bool isExist(const char * i_sectionKey) const throw();

        /** return string value by section and key or deafult value if not found. */
        string strValue(const char * i_section, const char * i_key, const string & i_default = "") const throw();

        /** return string value by section.key or deafult value if not found. */
        string strValue(const char * i_sectionKey, const string & i_default = "") const throw();

    private:
        bool is_loaded;         // if true then ini-file loaded OK else error
        IniEntryVec entryVec;   // ini-file entries: (section, key, value)

        // find index of section and key or -1 if not found
        ptrdiff_t findIndex(const char * i_section, const char * i_key) const;

        // find index of section.key or -1 if not found
        ptrdiff_t findIndex(const char * i_sectionKey) const;
    };
}

#endif  // OM_FILE_H
