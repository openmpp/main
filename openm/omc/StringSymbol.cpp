/**
* @file    StringSymbol.cpp
* Definitions for the StringSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "StringSymbol.h"

using namespace std;

void StringSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
    {
        // add this to the complete list of strings
        pp_all_strings.push_back(this);
        break;
    }
    default:
        break;
    }
}
