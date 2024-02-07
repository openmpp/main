/**
* @file    ModuleSymbol.cpp
* Definitions for the ModuleSymbol class.
*/
// Copyright (c) 2024-2024 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "omc_file.h"
#include "ModuleSymbol.h"
#include "LanguageSymbol.h"

using namespace std;

void ModuleSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignLabel:
    {
        assert(provenance != unknown); // assigned before post-parse passes
        // Unlike modgen, in ompp ModuleSymbol name includes extension.
        // For backwards compatibility, look for LABEL and NOTE using stem of module name.
        string stem = omc::getPathStem(name);
        associate_explicit_label_or_note(stem);
        break;
    }
    case ePopulateCollections:
    {
        // add this to the complete list of modules
        pp_all_modules.push_back(this);
        break;
    }
    default:
        break;
    }
}
