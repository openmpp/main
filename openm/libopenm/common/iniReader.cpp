/**
 * @file
 * OpenM++ ini-file reader: load ini-file and return values.
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "libopenm/common/omFile.h"
#include "log.h"
#include "iniReader.h"

using namespace std;
using namespace openm;

/** load all ini-file entries in memory. */
IniFileReader::IniFileReader(const char * i_filePath) : is_loaded(false)
{
    try {
        if (i_filePath == NULL || i_filePath[0] == '\0') return;    // nothing to do: empty ini-file name

        // read ini-file into UTF-8 string
        string fileContent = fileToUtf8(i_filePath);

        // parse file content into vector of entries
        string::size_type nStart = 0;
        string section;

        while (nStart < fileContent.length()) {

            // get the line
            string::size_type nextPos = fileContent.find_first_of("\r\n", nStart);
            if (nextPos != string::npos) nextPos = fileContent.find_first_not_of("\r\n", nextPos);
            if (nextPos == string::npos) nextPos = fileContent.length();

            string sLine = trim(fileContent.substr(nStart, nextPos - nStart));
            nStart = nextPos;   // to start of the next line

            // skip empty lines and ; commnets or # Linux comments
            if (sLine.length() < 2) continue;                   // at least k= or [] expected
            if (sLine[0] == ';' || sLine[0] == '#') continue;

            // check for the [section] ; and comment (optional)
            if (sLine[0] == '[') {

                string::size_type nEnd = sLine.find(']');   // [[potential end] of section name]

                if (nEnd != string::npos) {
                    string::size_type nRem = sLine.find_first_of(";#", nEnd);           // start of the ; comment
                    section = (nRem != string::npos) ? sLine.substr(0, nRem) : sLine;   // [SectionName]

                    continue;   // done with section header
                }
            }

            if (section.empty()) continue;  // if no [section] found then skip until first section

            // get key and value
            string::size_type nEq = sLine.find('=');
            if (nEq == string::npos) continue;          // skip invalid line: expected at least key=

            string key = sLine.substr(0, nEq);
            string val = (nEq + 1 < sLine.length()) ? sLine.substr(nEq + 1) : "";

            // add to the entry list or replace existing entry with new value
            IniEntry entry(section, key, val);
        
            IniEntryVec::const_iterator entryIt = std::find_if(
                entryVec.cbegin(), 
                entryVec.cend(), 
                bind(mem_fn(&IniEntry::equalTo), placeholders::_1, entry)
                );

            if (entryIt != entryVec.cend()) {
                entryVec[entryIt - entryVec.cbegin()].val = entry.val;
            }
            else {
                entryVec.push_back(entry);
            }
        }

        is_loaded = true;   // ini-file loaded correctly
    }
    catch (HelperException & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw;
    }
    catch (exception & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw HelperException(ex.what());
    }
}

// find index of section and key or -1 if not found
ptrdiff_t IniFileReader::findIndex(const char * i_section, const char * i_key) const
{
    // invalid section or key or file is not loaded
    if (!is_loaded || i_section == NULL || i_key == NULL) return -1;

    // search entry by section and key
    IniEntry entry(i_section, i_key, "");
        
    IniEntryVec::const_iterator entryIt = std::find_if(
        entryVec.cbegin(), 
        entryVec.cend(), 
        bind(mem_fn(&IniEntry::equalTo), placeholders::_1, entry)
        );

    // return index or -1 if not found
    return (entryIt != entryVec.cend()) ? entryIt - entryVec.cbegin() : -1;
}

// find index of section.key or -1 if not found
ptrdiff_t IniFileReader::findIndex(const char * i_sectionKey) const
{
    // invalid section or key or file is not loaded
    if (!is_loaded || i_sectionKey == NULL || i_sectionKey[0] == '\0') return -1;

    // search entry by section.key
    for (IniEntryVec::const_iterator entryIt = entryVec.cbegin(); entryIt != entryVec.cend(); entryIt++) {
        if (entryIt->bySectionKey(i_sectionKey)) return entryIt - entryVec.cbegin();
    }
    return -1;  // return -1 if not found
}

/** return true if section and key exist in ini-file. */
bool IniFileReader::isExist(const char * i_section, const char * i_key) const throw()
{
    try {
        return is_loaded ? findIndex(i_section, i_key) != -1 : false;
    }
    catch (...) { 
        return false;
    }
}

/** return true if section.key exist in ini-file. */
bool IniFileReader::isExist(const char * i_sectionKey) const throw()
{
    try {
        return is_loaded ? findIndex(i_sectionKey) != -1 : false;
    }
    catch (...) { 
        return false;
    }
}

/** return string value by section and key or deafult value if not found. */
string IniFileReader::strValue(const char * i_section, const char * i_key, const string & i_default) const throw()
{
    try {
        if (!is_loaded) return i_default;   // ini-file not loaded

        ptrdiff_t nPos = findIndex(i_section, i_key);
        return 
            (nPos >= 0) ? entryVec[nPos].val : i_default;
    }
    catch (...) { 
        return i_default;
    }
}

/** return string value by section.key or deafult value if not found. */
string IniFileReader::strValue(const char * i_sectionKey, const string & i_default) const throw()
{
    try {
        if (!is_loaded) return i_default;   // ini-file not loaded

        ptrdiff_t nPos = findIndex(i_sectionKey);
        return 
            (nPos >= 0) ? entryVec[nPos].val : i_default;
    }
    catch (...) { 
        return i_default;
    }
}

/**
 * copy values of section into map.
 *
 * @param[in]     i_section             section name
 * @param[in,out] io_dst                destination map to insert section values
 * @param[in]     i_isOverrideExisting  if true then override already existing values of section.key
 */
void IniFileReader::copySection(const char * i_section, NoCaseMap & io_dst, bool i_isOverrideExisting) const throw()
{ 
    try {
        // invalid section or file is not loaded
        if (!is_loaded || i_section == NULL || i_section[0] == '\0') return;

        for (const IniEntry & ent : entryVec) {
            if (!equalNoCase(i_section, ent.section.c_str())) continue;     // skip other sections

            // insert into map if override specified or section.key not already exists
            string sectionKey = string(i_section) + "." + ent.keySource;

            if (i_isOverrideExisting || io_dst.find(sectionKey) == io_dst.cend()) io_dst[sectionKey] = ent.val;
        }
    }
    catch (...) {
        return;
    }
}

/** initialize ini-file entry. */
IniEntry::IniEntry(const string & i_sectionSource, const string & i_keySource, const string & i_value)
{
    sectionSource = trim(i_sectionSource);
    keySource = trim(i_keySource);
    setSection();
    setKey();
    if (!i_value.empty()) setValue(i_value);
}

// set section name by source: remove [brackets], convert to lower case and trim
void IniEntry::setSection(void)
{
    string::size_type len = sectionSource.length();
    section = (len >= 2 && sectionSource[0] == '[' && sectionSource[len - 1] == ']') ? sectionSource.substr(1, len - 2) : sectionSource;
    toLower(section);
    section = trim(section);
}

// set key name by keySource: remove = if present, convert to lower and trim
void IniEntry::setKey(void)
{
    string::size_type len = keySource.length();
    key = (len >= 1 && keySource[len - 1] == '=') ? keySource.substr(0, len -1) : keySource;
    toLower(key);
    key = trim(key);
}

// set value: space trimed, comment removed and unquoted ("quotes" or 'apostrophes' removed)
// 
// rem = ; comment only and no value
// val = no comments
// nul =
// dsn = "DSN='server'; UID='user'; PWD='secret';"      ; comment: database connection string
// lst = "the # quick" brown 'fox # jumps ; over'       # comment: list of test words
// unb = "unbalanced quote                              ; comment: just in case
// 
void IniEntry::setValue(const string & i_value)
{
    val = trim(i_value);    // trim source value

    // search for begin of the comment outside of "quotes" or 'apostrophes'
    bool isInside = false;
    char chQuote = '\0';
    string::size_type quotePos = 0;
    string::size_type remPos = 0;

    for (string::size_type pos = 0; pos < val.length(); pos++) {

        char chNow = val[pos];

        // if comment char found outside of the quotes
        if (!isInside && (chNow == ';' || chNow == '#')) {
            val = val.substr(0, pos);
            val = trim(val);
            break;          // done: comment removed
        }

        // if comment char found inside of the quotes then save position to detect unbalanced quotes
        if (isInside && (chNow == ';' || chNow == '#')) {
            remPos = pos;
            continue;       // save start of the comment position inside of the quotes
        }

        // check for begin of the "quotes" or 'apostrophes'
        if (!isInside && (chNow == '"' || chNow == '\'')) {
            isInside = true;
            chQuote = chNow;
            quotePos = pos;
            continue;       // done with quote opening
        }
    
        // check for end of the "quotes" or 'apostrophes'
        if (isInside && chNow == chQuote) {
            isInside = false;
            chQuote = '\0';
            quotePos = pos;
            continue;       // done with quote closing
        }
    }
    // if unbalanced "quotes or 'apostrophes and also ; comment found then strip the comment
    if (isInside && remPos > quotePos) {
        val = val.substr(0, remPos);
        val = trim(val);
    }

    // remove value "quotes" or 'apostrophes' if present
    string::size_type len = val.length();
    if (len >= 2 && (val[0] == '"' || val[0] == '\'') && val[len - 1] == val[0]) val = val.substr(1, len - 2);
}

/** ini-file entry section and key case neutral equal comparison. */
bool IniEntry::equalTo(const IniEntry & i_other) const 
{ 
    return 
        equalNoCase(section.c_str(), i_other.section.c_str()) && 
        equalNoCase(key.c_str(), i_other.key.c_str());
}

/** ini-file entry section.key case neutral equal comparison. */
bool IniEntry::bySectionKey(const char * i_sectionKey) const
{
    return i_sectionKey != NULL && equalNoCase((section + "." + key).c_str(), i_sectionKey);
}
