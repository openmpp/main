/**
* @file    ModuleSymbol.cpp
* Definitions for the ModuleSymbol class.
*/
// Copyright (c) 2024-2024 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "ModuleSymbol.h"

using namespace std;

void ModuleSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        break;
    }
    case ePopulateCollections:
    {
        // add this to the complete list of modules
        //pp_all_imports.push_back(this);
        break;
    }
    default:
        break;
    }
}
