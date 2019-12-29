/**
* @file    ParameterGroupSymbol.cpp
* Definitions for the ParameterGroupSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

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

void ParameterGroupSymbol::populate_metadata(openm::MetaModelHolder& metaRows)
{
    {
        // For testing, dump each parameter group to omc log
        ostringstream ss;
        ss << "parameter group: "
            << name << ":\n";
        for (auto sym : pp_symbol_list) {
            bool is_group = dynamic_cast<GroupSymbol*>(sym);
            ss << "   "
                << (is_group ? "group: " : "param: ")
                << sym->name
                << "\n";
        }
        theLog->logMsg(ss.str().c_str());
    }
}
