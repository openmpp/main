/**
* @file    LanguageSymbol.cpp
* Definitions for the LanguageSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "LanguageSymbol.h"

using namespace std;

void LanguageSymbol::post_parse(int pass)
{
    // Hook into the hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
        {
            // Add this language to the complete list of languages.
            pp_all_languages.push_back(this);
        }
        break;
    default:
        break;
    }
}


int LanguageSymbol::next_language_id = 0;
