/**
* @file    TypeSymbol.cpp
* Definitions for the TypeSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "TypeSymbol.h"

using namespace std;

void TypeSymbol::post_parse(int pass)
{
    // Hook into the hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case 1:
        // add this type to the complete list of types
        pp_all_types.push_back(this);
        break;
    default:
        break;
    }
}


int TypeSymbol::next_type_id = 0;
