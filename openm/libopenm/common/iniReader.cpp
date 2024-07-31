/**
 * @file
 * OpenM++ ini-file reader: load ini-file and return values.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/omError.h"
#include "libopenm/common/omFile.h"
#include "libopenm/common/omOS.h"
#include "libopenm/common/iniReader.h"
#include "log.h"

using namespace std;
using namespace openm;

/** load all ini-file entries in memory and convert into UTF-8. 
*
* @param[in] i_filePath     path to ini-file.
* @param[in] i_codePageName (optional) name of encoding or Windows code page, ie: English_US.1252
* 
* Ini file example: 
* 
* [section]     ; section is required, global entries are not allowed
*               # this is also a comment
* rem =         ; comment only and empty value
*               ; next line is empty value without comment
* non =
* val = no comments
* dsn = "DSN='server'; UID='user'; PWD='secret';"   ; comment: database connection string
* lst = "the # quick" brown 'fox # jumps ; over'    # use "quote" and 'apostrophe' to escape characters and keep spaces
* unb = "unbalanced quote                           ; this is not a comment: it is a value started from " quote
*
* trim = Aname,Bname, \     ; comment: multi-line value joined with spaces trimmed
*        Cname,DName        ; comment: result is: Aname,Bname,Cname,DName
*
* abcd = Aname,Bname, \     ; multi-line value ended with empty line
*        Cname,DName  \     ; result is: 
*                           ; Aname,Bname,Cname,DName
*
* ; multi-line value started with " quote or ' apostrophe
* ; right spaces before \ is not trimmed, result is:
* ; Multi line   text with spaces
* ;
* keep = "Multi line   \
*        text with spaces"
*
* ; multi-line value started with " quote or ' apostrophe
* ; result is the same as above:
* ; Multi line   text with spaces
* ;
* same = "\
*        Multi line   \
*        text with spaces\
*        "
*/
IniFileReader::IniFileReader(const char * i_filePath, const char * i_codePageName) : is_loaded(false)
{
    // load ini-file using case neutral key comparison
    // if there are duplicated case-neutral keys then result is not defined
    // for example result is unpredictable if there are:
    //   KEY = abc
    //   key = def
    entryVec = load(i_filePath, true, i_codePageName);
    is_loaded = true;   // no exceptions, ini-file loaded correctly
}

/** load all ini-file entries in memory and convert into UTF-8 and return true if there are no exceptions during ini-file load.
*
* @param[in]  i_filePath      path to ini-file.
* @param[in]  is_noCase       if true then keys are case neutral, e.g. KEY = abc is the same as key = def
* @param[in]  i_codePageName  name of encoding or Windows code page, ie: English_US.1252
*/
const IniFileReader::IniEntryVec IniFileReader::load(const char * i_filePath, bool is_noCase, const char * i_codePageName)
{
    IniEntryVec eVec;
    try {
        if (i_filePath == NULL || i_filePath[0] == '\0') return eVec;    // nothing to do: empty ini-file name

        // read ini-file into UTF-8 string
        string fileContent = fileToUtf8(i_filePath, i_codePageName);
        string::size_type fileLen = fileContent.length();

        // parse file content into vector of entries
        string::size_type nStart = 0;
        int nLine = 1;
        int nNext = nLine;
        string sLine;
        string section;
        string sKey;
        string sValue;
        bool isContinue = false;

        while (nStart < fileLen) {

            // get the line, find start of next line and count line numbers by counting \n
            string::size_type nextPos = fileContent.find_first_of("\r\n", nStart);

            nLine = nNext;
            if (nextPos == string::npos) {
                nextPos = fileLen;
            }
            else {
                while (nextPos < fileLen) {
                    char c = fileContent[++nextPos];
                    if (c != '\r' && c != '\n') break;
                    if (c == '\n') nNext++;
                }
            }

            sLine = trim(fileContent.substr(nStart, nextPos - nStart));
            nStart = nextPos;   // to start of the next line

            // skip empty lines
            // if this is line continuation and key not empty then add new ini-entry
            string::size_type nLen = sLine.length();
            if (nLen <= 0) {
                if (isContinue && !sKey.empty()) {

                    addIniEntry(is_noCase, sLine, nLine, section, sKey, sValue, eVec);

                    sKey = "";
                    sValue = "";
                    isContinue = false;
                }
                continue;   // skip empty lines
            }

            // remove ; comments or # Linux comments:
            //   comment starts with ; or # outside of "quote" or 'single quote'
            // find the key:
            //   find first = outside of "quote" or 'single quote'
            bool isIn = false;
            char cq = '\0';
            string::size_type nRem = nLen + 1;
            string::size_type nEq = nLen + 1;

            for (string::size_type n = 0; n < nLen; n++) {

                char c = sLine[n];

                // if this is quote then toogle 'inside '' quote' status
                if ((!isIn && (c == '"' || c == '\'')) || (isIn && c == cq)) {
                    isIn = !isIn;
                    cq = isIn ? c : '\0';   // store openig quote or clear it on close
                    continue;               // done with quote char
                }

                // if not 'inside' of quoted key then check for ; or # as start of the comment
                if (!isIn) {
                    if (c == '=' && nEq >= nLen) nEq = n;   // found end of key=

                    if (c == ';' || c == '#') {
                        nRem = n;
                        break;      // found start of the comment
                    }
                }
            }
            // skip line: it is a comment only line
            if (nRem == 0) {
                if (isContinue && !sKey.empty()) {

                    addIniEntry(is_noCase, sLine, nLine, section, sKey, sValue, eVec);

                    sKey = "";
                    sValue = "";
                    isContinue = false;
                }
                continue;   // skip line: it is a comment only line
            }

            if (nRem < nLen) {          // remove comment from the end of the line
                sLine = trimRight(sLine.substr(0, nRem));
                nLen = sLine.length();
            }
            if (nLen <= 0) {
                if (isContinue && !sKey.empty()) {

                    addIniEntry(is_noCase, sLine, nLine, section, sKey, sValue, eVec);

                    sKey = "";
                    sValue = "";
                    isContinue = false;
                }
                continue;    // skip empty line (unexpected)
            }

            // check for the [section]
            // section is not allowed after previous line \ continuation
            // section cannot have empty [] name
            if (sLine[0] == '[') {

                if (isContinue) {
                    theLog->logFormatted("Skip incorrect section header as line continuation, ini file line %d: %s", nLine, sLine.c_str());
                    continue;
                }
                if (sLine[nLen - 1] != ']') {   // incorrect line: [section] expected
                    theLog->logFormatted("Skip incorrect ini file line %d: %s", nLine, sLine.c_str());
                    continue;
                }

                // set section name: remove [brackets] and trim
                section = trim(sLine.substr(1, nLen - 2));
                if (section.empty()) {
                    theLog->logFormatted("Skip incorrect (empty) section header at line %d of ini file: %s", nLine, sLine.c_str());
                }
                continue;       // done with section header
            }
            // skip incorrect line: before the first [section] only comments are allowed
            if (section.empty()) {
                theLog->logFormatted("Skip incorrect line %d of ini file: %s", nLine, sLine.c_str());
                continue;
            }

            // get key and value or continuation of previous line value
            string v = "";

            if (isContinue) {
                v = sLine;
            }
            else {
                // line must start from key=
                if (nEq <= 0 || nEq >= nLen) {
                    theLog->logFormatted("Skip incorrect line %d of ini file: %s", nLine, sLine.c_str());
                    continue;
                }

                sKey = sLine.substr(0, nEq);
                v = (nEq + 1 < nLen) ? trimLeft(sLine.substr(nEq + 1)) : "";
            }

            // check if there is a continuation \ at the end of current line
            // right trim continuation line if value not started with " quote or ' apostrophe
            if (string::size_type lv = v.length(); lv <= 0 || v[lv - 1] != '\\') {
                isContinue = false;
            }
            else {
                isContinue = true;  // next line is a value continuation

                string::size_type n = sValue.length();
                bool isFirstQuote =
                    (n > 0 && (sValue[0] == '"' || sValue[0] == '\'')) ||
                    (n <= 0 && (v[0] == '"' || v[0] == '\''));

                v = isFirstQuote ? v.substr(0, lv - 1) : trimRight(v.substr(0, lv - 1));
            }
            sValue = sValue + v;        // append current line value to result value

            if (isContinue) continue;   // done with this line, next line is a value continuation

            // add new ini-entry, if it is a valid (if section and key are not empty)
            addIniEntry(is_noCase, sLine, nLine, section, sKey, sValue, eVec);

            sKey = "";
            sValue = "";
            isContinue = false;
        }

        // process last line ini-file end with line continuation without cr-lf
        // if this is line continuation and key not empty then add new ini-entry
        if (isContinue && !sKey.empty()) {
            addIniEntry(is_noCase, sLine, nLine, section, sKey, sValue, eVec);
        }
    }
    catch (HelperException & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw;
    }
    catch (exception & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw HelperException(ex.what());
    }
    return eVec;
}

// insert new or update existing ini-file entry:
//  unquote key and value if "quoted" or 'single quoted'
//  return false on error: if section or key is empty
bool IniFileReader::addIniEntry(bool is_noCase, const string & i_src, int i_nLine, const string & i_section, const string & i_key, const string & i_value, IniEntryVec & o_entryVec)
{

    // make new entry, unquote key and value if "quoted" or 'single quoted'
    IniEntry newEntry(i_section, i_key, i_value);
    if (newEntry.section.empty() || newEntry.key.empty()) {     // skip empty sections or empty keys
        theLog->logFormatted("Skip incorrect empty section or empty key at line %d of ini file: %s", i_nLine, i_src.c_str());
        return false;
    }

    // add to the entry list or replace existing entry with new value
    IniEntryVec::const_iterator entryIt = std::find_if(
        o_entryVec.cbegin(),
        o_entryVec.cend(),
        [&is_noCase, &newEntry, &i_section, &i_key](const IniEntry & i_entry) -> bool {
            return is_noCase ?
                i_entry.equalTo(newEntry.section.c_str(), newEntry.key.c_str()) :
                i_entry.section == i_section && i_entry.key == i_key;
        }
    );
    if (entryIt != o_entryVec.cend()) {
        o_entryVec[entryIt - o_entryVec.cbegin()].val = newEntry.val;
    }
    else {
        o_entryVec.push_back(newEntry);
    }

    return true;
}

// find index of section and key or -1 if not found
ptrdiff_t IniFileReader::findIndex(const char * i_section, const char * i_key) const
{
    // invalid section or key or file is not loaded
    if (!is_loaded || i_section == NULL || i_key == NULL) return -1;

    // search entry by section and key
    IniEntryVec::const_iterator entryIt = std::find_if(
        entryVec.cbegin(),
        entryVec.cend(),
        [i_section, i_key](const IniEntry & entry) -> bool { return entry.equalTo(i_section, i_key); }
    );

    // return index or -1 if not found
    return (entryIt != entryVec.cend()) ? entryIt - entryVec.cbegin() : -1;
}

// find index of section.key or -1 if not found
ptrdiff_t IniFileReader::findIndex(const char * i_sectionKey) const
{
    // invalid section or key or file is not loaded
    if (!is_loaded || i_sectionKey == NULL || i_sectionKey[0] == '\0') return -1;

    // search entry by section and key
    IniEntryVec::const_iterator entryIt = std::find_if(
        entryVec.cbegin(),
        entryVec.cend(),
        [i_sectionKey](const IniEntry & entry) -> bool { return entry.bySectionKey(i_sectionKey); }
    );

    // return index or -1 if not found
    return (entryIt != entryVec.cend()) ? entryIt - entryVec.cbegin() : -1;
}

/** return true if section and key exist in ini-file. */
bool IniFileReader::isExist(const char * i_section, const char * i_key) const noexcept
{
    try {
        return is_loaded ? findIndex(i_section, i_key) != -1 : false;
    }
    catch (...) { 
        return false;
    }
}

/** return true if section.key exist in ini-file. */
bool IniFileReader::isExist(const char * i_sectionKey) const noexcept
{
    try {
        return is_loaded ? findIndex(i_sectionKey) != -1 : false;
    }
    catch (...) { 
        return false;
    }
}

/** return string value by section and .key using case-neutral search, or default value if not found. */
const string IniFileReader::strValue(const char * i_section, const char * i_key, const string & i_default) const noexcept
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

/** return string value by section.key using case-neutral search, or default value if not found. */
const string IniFileReader::strValue(const char * i_sectionKey, const string & i_default) const noexcept
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

/** return names of ini-file sections as case-neutral set of strings. */
const NoCaseSet IniFileReader::sectionSet(void) const noexcept
{
    try {
        NoCaseSet rs;
        for (const auto & e : entryVec) {
            rs.insert(e.section);
        }
        return rs;
    }
    catch (...) { 
        return NoCaseSet();
    }
}

/** return section by name as case-neutral map of (key,value). */
const NoCaseMap IniFileReader::getSection(const char * i_section) const noexcept
{
    try {
        NoCaseMap sectMap;

        if (!is_loaded || i_section == NULL || i_section[0] == '\0') return sectMap;    // empty section name or file is not loaded

        for (const IniEntry & ent : entryVec) {
            if (equalNoCase(i_section, ent.section.c_str())) sectMap[ent.key] = ent.val;
        }
        return sectMap;
    }
    catch (...) {
        return NoCaseMap();
    }
}

/** initialize ini-file entry. */
IniEntry::IniEntry(const string & i_section, const string & i_key, const string & i_value)
{
    section = i_section;
    setKey(i_key);
    if (!i_value.empty()) setValue(i_value);
}

// set key name: remove = if present, unquote and trim
void IniEntry::setKey(const string & i_key)
{
    key = trim(i_key);
    string::size_type len = key.length();
    if (len >= 1 && key[len - 1] == '=') key = key.substr(0, len - 1);
    key = trim(key);
    len = key.length();
    if (len >= 2 && (key[0] == '"' || key[0] == '\'') && key[len - 1] == key[0]) key = key.substr(1, len - 2);
}

// set value: trim space and remove "quotes" or 'apostrophes'
void IniEntry::setValue(const string & i_value)
{
    val = trim(i_value);    // trim source value

    // remove value "quotes" or 'apostrophes' if present
    string::size_type len = val.length();
    if (len >= 2 && (val[0] == '"' || val[0] == '\'') && val[len - 1] == val[0]) val = val.substr(1, len - 2);
}

/** ini-file entry section.key using case neutral equal comparison. */
bool IniEntry::bySectionKey(const char * i_sectionKey) const
{
    return i_sectionKey != nullptr && equalNoCase((section + "." + key).c_str(), i_sectionKey);
}

/** ini-file entry section and key using case neutral equal comparison. */
bool IniEntry::equalTo(const char * i_section, const char * i_key) const
{
    return i_section != nullptr && i_key != nullptr && equalNoCase(section.c_str(), i_section) && equalNoCase(key.c_str(), i_key);
}

/** read language specific messages from path/to/theExe.message.ini and pass it to the log */
void IniFileReader::loadMessages(const char * i_iniMsgPath, const string & i_language, const char * i_codePageName) noexcept
{
    try {
        // get list of user prefered languages, if user language == en_CA.UTF-8 then list is: (en-ca, en)
        list<string> langLst = splitLanguageName(!i_language.empty() ? i_language : getDefaultLocaleName());

        // set user prefered language(s)
        unordered_map<string, string> msgMap;
        theLog->swapLanguageMessages(langLst, msgMap);

        // read modelName.message.ini
        if (!isFileExists(i_iniMsgPath)) return;     // exit: message.ini does not exists

        // load ini-file using case sensitive key comparison
        IniEntryVec eIniVec = load(i_iniMsgPath, false, i_codePageName);

        // find user language(s) as section of message.ini and copy messages into message map
        // translated message is searched in language prefered order: (en-ca, en)

        for (const string & lang : langLst) {   // search in order of user prefered languages: (en-ca, en)

            for (const auto & e : eIniVec) {

                // if section is not a current language then skip message.ini entry
                if (!equalNoCase(lang.c_str(), normalizeLanguageName(e.section).c_str())) continue;

                // add translated messages, if not already in message map
                // use only translated messages (where translated value is not empty)
                if (!e.key.empty() && !e.val.empty() && msgMap.find(e.key) == msgMap.end()) msgMap[e.key] = e.val;
            }
        }

        // pass translated messages to the logger
        theLog->swapLanguageMessages(langLst, msgMap);
    }
    catch (HelperException & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        // throw;                               = exit without failure
    }
    catch (exception & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        // throw HelperException(ex.what());    = exit without failure
    }
}

/** read language specific messages from path/to/theExe.message.ini for all languages */
list<pair<string, unordered_map<string, string>>> IniFileReader::loadAllMessages(const char * i_iniMsgPath, const char * i_codePageName) noexcept
{
    try {
        // read modelName.message.ini
        if (!isFileExists(i_iniMsgPath)) return list<pair<string, unordered_map<string, string>>>();    // exit: message.ini does not exists

        // load ini-file using case sensitive key comparison
        IniEntryVec eIniVec = load(i_iniMsgPath, false, i_codePageName);

        NoCaseSet sectSet;
        for (const auto & e : eIniVec) {
            sectSet.insert(e.section);
        }

        // for each language find section of message.ini and copy messages into message map
        // 
        //   loop over the list of languages in descending order, ex: fr-CA fr en-CA en
        //   it is an attempt to get the message in more specific language before more generic: in fr-CA and not in fr
        //   this is a very simplistic approach and result may be incorrect, e.g. en-US is always prefered to en-CA
        list<pair<string, unordered_map<string, string>>> allMsg;

        for (auto langSecIt = sectSet.crbegin(); langSecIt != sectSet.crend(); ++langSecIt) {

            unordered_map<string, string> msgMap;

            for (const auto & e : eIniVec) {

                // if section is not a current language then skip message.ini entry
                if (!equalNoCase(langSecIt->c_str(), normalizeLanguageName(e.section).c_str())) continue;

                // add translated messages, if not already in message map
                // use only translated messages (where translated value is not empty)
                if (!e.key.empty() && !e.val.empty() && msgMap.find(e.key) == msgMap.end()) msgMap[e.key] = e.val;
            }
            if (!msgMap.empty()) {
                allMsg.push_back(pair<string, unordered_map<string, string>>(*langSecIt, msgMap));
            }
        }

        return allMsg;
    }
    catch (HelperException & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        // throw;                               = exit without failure
    }
    catch (exception & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        // throw HelperException(ex.what());    = exit without failure
    }
    return list<pair<string, unordered_map<string, string>>>(); // retrun empty value on error
}
