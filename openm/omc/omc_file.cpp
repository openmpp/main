/**
* @file    omc_helper.cpp
 * omc file helper functions.
*/
// Copyright (c) 2013-2016 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "omc_common.h"
#include "omc_file.h"

using namespace std;
using namespace openm;

// get list of files matching extension list from specified directory or current directory if source path is empty
// each file name in result is a relative path and include source directory
list<string> omc::listSourceFiles(const string & i_srcPath, const list<string> & i_extensions)
{
    list<string> pathLst;
    
    // open source directory or current directory if source path is empty
    string srcPath = !i_srcPath.empty() ? i_srcPath : ".";
    DIR * dir = opendir(srcPath.c_str());
    if (dir == NULL) throw HelperException(LT("error : cannot open source directory %s"), srcPath.c_str());

    // collect list of .mpp, .ompp or .dat files
    try {
        dirent * ent;
        struct stat fileStat;
        string path;

        string basePath = i_srcPath;
        if (!basePath.empty() && basePath.back() != '/' && basePath.back() != '\\') basePath += '/';

        while ((ent = readdir(dir)) != NULL) {

            if (ent->d_name == NULL) continue;  // skip file name errors

            path = basePath + ent->d_name;     // include directory into result

            // skip directories and special files
            if (ent->d_type != DT_REG) {
                if (ent->d_type != DT_UNKNOWN) continue;            // skip directories and special files
                if (stat(path.c_str(), &fileStat) != 0) continue;   // we can't get file type
                if (!S_ISREG(fileStat.st_mode)) continue;           // skip directories and special files
            }

            for (auto ext : i_extensions) {
                if (endWithNoCase(path, ext.c_str())) {
                    pathLst.push_back(path);
                    break;
                }
            }
        }
    }
    catch(...) {
        closedir(dir);  // close directory on error
        throw;
    }
    
    // sort source files in alphabetical order for reproducibility
    pathLst.sort();
    return pathLst;
}

// get extension of filename
string omc::getFileNameExt(const string &file_name)
{
    // length of optional path part, including final trailing slash
    int in_path_len = file_name.find_last_of("/\\");
    if (in_path_len == -1) in_path_len = 0;
    if (in_path_len > 0) in_path_len++;

    // position of stem part
    int in_stem_pos = in_path_len;

    // position of extension (0 if no extension)
    int in_ext_pos = file_name.find_last_of(".");
    if (in_ext_pos < in_path_len) in_ext_pos = 0; // ignore final . if in path portion

    // length of extension
    int in_ext_len = (in_ext_pos == 0) ? 0 : file_name.length() - in_ext_pos;

    // length of stem
    int in_stem_len = file_name.length() - in_path_len - in_ext_len;

    string in_path = file_name.substr(0, in_path_len);
    string in_stem = file_name.substr(in_stem_pos, in_stem_len);
    string in_ext = file_name.substr(in_ext_pos, in_ext_len);
    openm::toLower(in_ext);

    return in_ext;
}

// get stem of filename
string omc::getFileNameStem(const string &file_name)
{
    // length of optional path part, including final trailing slash
    int in_path_len = file_name.find_last_of("/\\");
    if (in_path_len == -1) in_path_len = 0;
    if (in_path_len > 0) in_path_len++;

    // position of stem part
    int in_stem_pos = in_path_len;

    // position of extension (0 if no extension)
    int in_ext_pos = file_name.find_last_of(".");
    if (in_ext_pos < in_path_len) in_ext_pos = 0; // ignore final . if in path portion

    // length of extension
    int in_ext_len = (in_ext_pos == 0) ? 0 : file_name.length() - in_ext_pos;

    // length of stem
    int in_stem_len = file_name.length() - in_path_len - in_ext_len;

    string in_path = file_name.substr(0, in_path_len);
    string in_stem = file_name.substr(in_stem_pos, in_stem_len);
    string in_ext = file_name.substr(in_ext_pos, in_ext_len);
    openm::toLower(in_ext);

    return in_stem;
}

// create output/modelName.message.ini file by merging model messages and languages with existing code/modelName.message.ini 
void omc::buildMessageIni(
    const MetaModelHolder & i_metaRows,
    const string & i_inpDir,
    const string i_outDir,
    const char * i_codePageName,
    const unordered_set<string> & i_msgSet
)
{
    if (i_msgSet.empty()) return;   // exit: no messages to translate

    map<string, NoCaseMap, LessNoCase> iniMap;  // output ini-flie as case-netral map
    NoCaseSet langSet;
    locale loc("");         // current user locale

    // cleanup line breaks
    unordered_set<string> msgSet;
    for (string msg : i_msgSet) {
        blankCrLf(msg);
        msgSet.insert(msg);
    }

    // if exist code/modelName.message.ini then read it and 
    // for each language merge new model messages with existing message translations
    string srcPath = makeFilePath(i_inpDir.c_str(), i_metaRows.modelDic.name.c_str(), ".message.ini");

    if (isFileExists(srcPath.c_str())) {    // if exist code/modelName.message.ini

        // read existing translation file and get languages, which are section names
        IniFileReader rd(srcPath.c_str(), i_codePageName);
        langSet = rd.sectionSet();

        for (const auto & sect : langSet) {

            auto sectIt = iniMap.insert(pair<string, NoCaseMap>(sect, NoCaseMap()));
            NoCaseMap & ctMap = sectIt.first->second;

            for (const string & msg : msgSet) {
                ctMap[msg] = rd.strValue(sect.c_str(), msg.c_str());
            }
        }

        // BEGIN of optional code: can be commented
        // if required count deleted translations for each model language
        for (const LangLstRow & langRow : i_metaRows.langLst) {

            const auto sectIt = iniMap.find(trim(langRow.code, loc));   // language section in new ini-file
            if (sectIt == iniMap.end()) continue;                       // skip: no such model language

            NoCaseMap rdMap = rd.getSection(sectIt->first.c_str()); // section [language] => (code,translation)
            int nDel = 0;

            for (const auto & rd : rdMap) {
                if (sectIt->second.find(rd.first) == sectIt->second.end()) nDel++;
            }
            if (nDel > 0) theLog->logFormatted("Deleted %d translated message(s) from language %s", nDel, langRow.code.c_str());
        }
        // END of optional code
    }
    // merge done for existing ini-file messages

    // BEGIN of optional code: can be commented
    // do the counts and report translation status
    // assume no translations if only one model language
    if (i_metaRows.langLst.size() > 1) {

        // count missing model languages
        // count missing translations for each model language
        int nLangMissing = 0;   // count missing translations for each model language
        int nMsgMissing = 0;    // total count of count missing translations

        for (const LangLstRow & langRow : i_metaRows.langLst) {

            const auto sectIt = iniMap.find(trim(langRow.code, loc));   // language section in new ini-file
            if (sectIt == iniMap.end()) {
                nLangMissing++;
                theLog->logFormatted("Missing translated messages for language %s", langRow.code.c_str());
                continue;           // missing model language
            }

            int nMissing = 0;    // count missing translations for current model language

            for (const auto & ct : sectIt->second) {
                if (ct.second.empty()) nMissing++;      // message exist in in-file, but translation is empty
            }
            for (const string & msg : msgSet) {
                if (sectIt->second.find(msg) == sectIt->second.end()) nMissing++;   // message not exist in ini-file
            }
            nMsgMissing += nMissing;
            if (nMsgMissing > 0) theLog->logFormatted("Missing %d translated message(s) for language %s", nMsgMissing, langRow.code.c_str());
        }

        if (nMsgMissing > 0) theLog->logFormatted("Missing %d translated message(s)", nMsgMissing);
        if (nLangMissing > 0) theLog->logFormatted("Missing translated messages for %d language(s)", nLangMissing);
    }
    // END of optional code

    // for missing model languages append to output ini-file empty translations
    // assume no translations if only one model language
    if (i_metaRows.langLst.size() > 1) {

        for (const LangLstRow & langRow : i_metaRows.langLst) {

            // if language section exist in new ini-file then skip it
            string sect = trim(langRow.code, loc);
            if (iniMap.find(sect) != iniMap.end()) continue;

            auto sectIt = iniMap.insert(pair<string, NoCaseMap>(sect, NoCaseMap()));
            NoCaseMap & ctMap = sectIt.first->second;

            for (const string & msg : msgSet) { // insert empty translation
                ctMap[msg] = "";
            }
        }
    }

    // write updated version of output/modelName.message.ini
    if (!iniMap.empty()) {

        string dstPath = makeFilePath(i_outDir.c_str(), i_metaRows.modelDic.name.c_str(), ".message.ini");

        ofstream iniFs(dstPath, ios::out | ios::trunc | ios::binary);
        exit_guard<ofstream> onExit(&iniFs, &ofstream::close);   // close on exit
        if (iniFs.fail()) throw HelperException(LT("error : unable to open %s for writing"), dstPath.c_str());

        iniFs << "; Translated messages for model: " << i_metaRows.modelDic.name << "\r\n" <<
            "; \r\n" <<
            "; Each message must be on a single line, multi-line messages NOT supported \r\n" <<
            "; Each message must be Key = Value \r\n" <<
            "; If message starts or end with blank space then put it inside of \"quotes\" or 'single' quotes \r\n" <<
            "; Part of the line after ; or # is just a comment \r\n" <<
            "; Examples: \r\n" <<
            "; some message = message in English                                    ; this part is a comment \r\n" <<
            "; \" space can be preserved \" = \" space can be preserved in English \" \r\n" <<
            "; ' you can use single quote ' = ' you can use single quote or '' combine \"quotes\" ' \r\n" <<
            "\r\n";

        for (const auto & iniSect : iniMap) {

            iniFs << "[" << iniSect.first << "]\r\n";
            if (iniFs.fail()) throw HelperException(LT("error : unable to write into: %s"), dstPath.c_str());

            for (const auto & ct : iniSect.second) {

                // cleanup: remove empty messages (empty message code)
                if (ct.first.empty()) continue;

                // if code or value start from or end with space
                // put "quotes" or 'single quotes' around of code and value
                if ((isspace<char>(ct.first.front(), loc) || isspace<char>(ct.first.back(), loc)) ||
                    (!ct.second.empty() && (isspace<char>(ct.second.front(), loc) || isspace<char>(ct.second.back(), loc)))) {
                
                    if (ct.first.find("\"") != string::npos) {
                        iniFs << "'" << ct.first << "' = '" << ct.second << "'\r\n";
                    }
                    else {
                        iniFs << "\"" << ct.first << "\" = \"" << ct.second << "\"\r\n";
                    }
                }
                else {
                    iniFs << ct.first << " = " << ct.second << "\r\n";
                }
                if (iniFs.fail()) throw HelperException(LT("error : unable to write into: %s"), dstPath.c_str());
            }

            iniFs << "\r\n";
            if (iniFs.fail()) throw HelperException(LT("error : unable to write into: %s"), dstPath.c_str());
        }
    }
}