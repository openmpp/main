/**
* @file    ImportSymbol.cpp
* Definitions for the ImportSymbol class.
*/
// Copyright (c) 2013-2019 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "ImportSymbol.h"

using namespace std;

void ImportSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        pp_target_param = dynamic_cast<ParameterSymbol*>(pp_symbol(target_param));
        if (!pp_target_param) {
            pp_error(LT("error : '") + target_param->name + LT("' is not a parameter"));
        }
        break;
    }
    case ePopulateCollections:
    {
        // add this to the complete list of imports
        pp_all_imports.push_back(this);
        break;
    }
    default:
        break;
    }
}

void ImportSymbol::populate_metadata(openm::MetaModelHolder& metaRows)
{
    {
        // For testing, dump each import to omc log
        ostringstream ss;
        ss << "import(test): "
            << pp_target_param->name << ","
            << upstream_model << ","
            << upstream_table << ","
            << (sample_dimension_opt ? 1 : 0);
        theLog->logMsg(ss.str().c_str());
    }
}

int ImportSymbol::counter = 0;
