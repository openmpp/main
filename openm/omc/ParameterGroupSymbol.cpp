/**
* @file    ParameterGroupSymbol.cpp
* Definitions for the ParameterGroupSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "ParameterGroupSymbol.h"

using namespace std;

void ParameterGroupSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
    {
        // add this to the complete list of parameter groups
        pp_all_parameter_groups.push_back(this);
        break;
    }
    default:
        break;
    }
}
