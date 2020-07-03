/**
 * @file    omc_file.h
 * omc file helper functions.
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
}
