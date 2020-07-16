/**
 * @file    omc_csv.h
 * omc functions to read parameters data from .csv or .tsv files.
 */
// Copyright (c) 2013-2016 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once

#include <forward_list>
#include "libopenm/omLog.h"
#include "libopenm/common/omFile.h"
#include "TypeSymbol.h"
#include "EnumerationSymbol.h"
#include "ParameterSymbol.h"

using namespace std;

namespace omc
{
    // read parameters data from .csv or .tsv files from fixed directory and parameter directory
    extern void readParameterCsvFiles(
        bool i_isFixed, 
        const string & i_srcDir, 
        const map<string, ParameterSymbol *> & i_paramNameMap, 
        forward_list<unique_ptr<Constant> > & io_cpLst
    );
}
