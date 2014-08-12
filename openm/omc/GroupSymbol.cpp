/**
* @file    GroupSymbol.cpp
* Definitions for the GroupSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "GroupSymbol.h"

using namespace std;

void GroupSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
    {
        // create post-parse collection of groups and symbols
        for (auto sym : symbol_list) {
            auto pp_sym = pp_symbol(sym);
            assert(pp_sym);
            pp_symbol_list.push_back(pp_sym);
        }
        break;
    }
    default:
        break;
    }
}
