/**
 * @file
 * OpenM++ common file utilities.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef OM_INI_READER_H
#define OM_INI_READER_H

#include "omFile.h"

using namespace std;

namespace openm
{
    /** Ini-file entry: setcion, key and value */
    struct IniEntry
    {
        /** section: in lower case, without [], space trimed */
        string section;

        /** key: in lower case, without =, space trimed */
        string key;

        /** value: space trimed and unquoted ("quotes" or 'apostrophes' removed) */
        string val;

        /**
         * initialize ini-file entry.
         *
         * @param[in] i_section ini-file section name.
         * @param[in] i_key     ini-file key.
         * @param[in] i_value   ini-file entry value.
         */
        IniEntry(const string & i_section, const string & i_key, const string & i_value);

        /** ini-file entry section.key case neutral equal comparison. */
        bool bySectionKey(const char * i_sectionKey) const;

        /** ini-file entry section and key case neutral equal comparison. */
        bool equalTo(const char * i_section, const char * i_key) const;

    private:
        // set section name: remove [brackets] and trim
        void setSection(const string & i_section);

        // set key name: remove = if present, unquote and trim
        void setKey(const string & i_key);

        // set value: space trimed, comment removed and unquoted ("quotes" or 'apostrophes' removed)
        void setValue(const string & i_value);
    };

    /** ini-file reader: load all entries in memory and provide search methods. */
    class IniFileReader
    {
        /** ini-file entries: vector of (section, key, value). */
        typedef vector<IniEntry> IniEntryVec;

    public:
        /** load all ini-file entries in memory and convert into UTF-8. 
        *
        * @param[in] i_filePath     path to ini-file.
        * @param[in] i_codePageName (optional) name of encoding or Windows code page, ie: English_US.1252
        */
        IniFileReader(const char * i_filePath, const char * i_codePageName = nullptr);

        /** return true if ini-file loaded correctly or false on error */
        bool isLoaded(void) const throw() { return is_loaded; }

        /** return true if section and key exist in ini-file. */
        bool isExist(const char * i_section, const char * i_key) const throw();

        /** return true if section.key exist in ini-file. */
        bool isExist(const char * i_sectionKey) const throw();

        /** return string value by section and key or deafult value if not found. */
        const string strValue(const char * i_section, const char * i_key, const string & i_default = "") const throw();

        /** return string value by section.key or deafult value if not found. */
        const string strValue(const char * i_sectionKey, const string & i_default = "") const throw();

        /** return names of ini-file sections as case-neutral set of strings. */
        const NoCaseSet sectionSet(void) const throw();

        /** return section by name as case-neutral map of (key,value). */
        const NoCaseMap getSection(const char * i_section) const throw();

        /** read language specific messages from path/to/theExe.message.ini and pass it to the log */
        static void loadMessages(const char * i_exePath) throw();

    private:
        bool is_loaded;         // if true then ini-file loaded OK else error
        IniEntryVec entryVec;   // ini-file entries: (section, key, value)

        // find index of section and key or -1 if not found
        ptrdiff_t findIndex(const char * i_section, const char * i_key) const;

        // find index of section.key or -1 if not found
        ptrdiff_t findIndex(const char * i_sectionKey) const;
    };
}

#endif  // OM_INI_READER_H
