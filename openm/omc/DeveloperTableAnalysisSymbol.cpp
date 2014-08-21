/**
* @file    DeveloperTableAnalysisSymbol.cpp
* Definitions for the DeveloperTableAnalysisSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "DeveloperTableAnalysisSymbol.h"

using namespace std;

void DeveloperTableAnalysisSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
    {
        break;
    }
    default:
        break;
    }
}
