/**
 * @file    omc_file.h
 * omc file helper functions and translation functions.
 */
// Copyright (c) 2013-2016 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

#include <iostream>
#include <fstream>
#include <list>
#include <unordered_set>
#include <filesystem>
#include "libopenm/omLog.h"
#include "libopenm/common/omFile.h"
#include "libopenm/common/iniReader.h"
#include "libopenm/db/dbMetaRow.h"
#include "libopenm/db/metaModelHolder.h"
#include "libopenm/db/metaSetHolder.h"

using namespace std;

namespace omc
{
    using namespace openm;

    // get list of files matching extension list from specified directory or current directory if source path is empty
    // each file name in result is a relative path and include source directory
    extern list<string> listSourceFiles(const string & i_srcPath, const list<string> & i_extensions);

    // get extension of filename
    extern string getFileNameExt(const string &file_name);

    // get stem of filename
    extern string getFileNameStem(const string &file_name);

    // create output/modelName.message.ini file by merging model messages and languages with existing code/modelName.message.ini 
    extern void buildMessageIni(
        const MetaModelHolder & i_metaRows,
        const string & i_inpDir,
        const string i_outDir,
        const char * i_codePageName,
        const unordered_set<string> & i_msgSet
    );

    /** storage of translated messages for all languages from omc.message.ini */
    class TranslatedStore
    {
    public:
        TranslatedStore() {};
        ~TranslatedStore(void) noexcept {};

        /** read translated strings for all model languages from omc.message.ini */
        void load(const string & i_msgFilePath) noexcept
        {
            allMsg = IniFileReader::loadAllMessages(i_msgFilePath.c_str());
        }

        /** find language-specific message by source non-translated message and language
        *
        * @param[in] i_lang   language to translate into, e.g.: fr-CA
        * @param[in] i_source source message to translate
        */
        const string getTranslated(const char * i_lang, const char * i_source) const noexcept;

    private:
        // list of langauges and for each language pairs of key and translated string
        list<pair<string, unordered_map<string, string>>> allMsg;

    private:
        TranslatedStore(const TranslatedStore & i_store) = delete;
        TranslatedStore & operator=(const TranslatedStore & i_store) = delete;
    };

    /** translated messages for all languages from omc.message.ini */
    extern TranslatedStore * theAllTranslated;
}

/**
 * @def LTA(lang, sourceMessage)
 *
 * @brief   LTA translation function: find translated string by language code and source message
 *          string.
 *
 * @param   lang            The language.
 * @param   sourceMessage   Message describing the source.
 */
#define LTA(lang, sourceMessage) ((theAllTranslated->getTranslated(lang.c_str(), sourceMessage)))
