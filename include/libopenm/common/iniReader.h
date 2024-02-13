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

        /** ini-file entry section.key using case neutral equal comparison. */
        bool bySectionKey(const char * i_sectionKey) const;

        /** ini-file entry section and key using case neutral equal comparison. */
        bool equalTo(const char * i_section, const char * i_key) const;

    private:
        // set key name: remove = if present, unquote and trim
        void setKey(const string & i_key);

        // set value: trim space and remove "quotes" or 'apostrophes'
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
        bool isLoaded(void) const noexcept { return is_loaded; }

        /** return true if section and key found in ini-file using case-neutral search. */
        bool isExist(const char * i_section, const char * i_key) const noexcept;

        /** return true if section.key found in ini-file using case-neutral search. */
        bool isExist(const char * i_sectionKey) const noexcept;

        /** return string value by section and key using case-neutral search, or default value if not found. */
        const string strValue(const char * i_section, const char * i_key, const string & i_default = "") const noexcept;

        /** return string value by section.key using case-neutral search, or default value if not found. */
        const string strValue(const char * i_sectionKey, const string & i_default = "") const noexcept;

        /** return const reference to ini-file entries. */
        const IniEntryVec & rowsCRef(void) const { return entryVec; }

        /** return names of ini-file sections as case-neutral set of strings. */
        const NoCaseSet sectionSet(void) const noexcept;

        /** return section by name as case-neutral map of (key,value). */
        const NoCaseMap getSection(const char * i_section) const noexcept;

        /** read language specific messages from path/to/theExe.message.ini and pass it to the log */
        static void loadMessages(const char * i_iniMsgPath, const string & i_language = "", const char * i_codePageName = nullptr) noexcept;

        /** read language specific messages from path/to/theExe.message.ini for all languages
        *
        * @param[in] i_iniMsgPath path to message ini-file, usually: path/to/theExe.message.ini
        */
        static list<pair<string, unordered_map<string, string>>> loadAllMessages(const char * i_iniMsgPath, const char * i_codePageName = nullptr) noexcept;

    private:
        bool is_loaded;         // if true then ini-file loaded OK else error
        IniEntryVec entryVec;   // ini-file entries: (section, key, value)

        /** load all ini-file entries in memory and convert into UTF-8, raise exception if read of ini-file failed.
        *
        * @param[in]  i_filePath      path to ini-file.
        * @param[in]  is_noCase       if true then keys are case neutral, e.g. KEY = abc is the same as key = def
        * @param[in]  i_codePageName  name of encoding or Windows code page, ie: English_US.1252
        * @param[out] o_entryVec      ini-file entries: (section, key, value)
        */
        static void load(const char * i_filePath, bool is_noCase, const char * i_codePageName, IniEntryVec & o_entryVec);

        // find index of section and key or -1 if not found
        ptrdiff_t findIndex(const char * i_section, const char * i_key) const;

        // find index of section.key or -1 if not found
        ptrdiff_t findIndex(const char * i_sectionKey) const;

        // insert new or update existing ini-file entry:
        //  unquote key and value if "quoted" or 'single quoted'
        //  return false on error: if section or key is empty
        static bool addIniEntry(
            bool is_noCase, const string & i_src, int i_nLine, const string & i_section, const string & i_key, const string & i_value, IniEntryVec & o_entryVec
        );
    };
}

#endif  // OM_INI_READER_H
