/**
* @file    ParameterGroupSymbol.cpp
* Definitions for the ParameterGroupSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "ParameterGroupSymbol.h"
#include "ParameterSymbol.h"

using namespace std;

void ParameterGroupSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // check that group members are allowed types
        // and check if all parameters are fixed
        for (auto sym : pp_symbol_list) {
            bool is_parameter = dynamic_cast<ParameterSymbol*>(sym);
            bool is_parameter_group = dynamic_cast<ParameterGroupSymbol*>(sym);
            if (!(is_parameter || is_parameter_group)) {
                pp_error(LT("error : invalid member '") + sym->name + LT("' of parameter group '") + name + LT("'"));
            }
        }
        break;
    }
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
    if (!contains_scenario_parameter()) {
        // do not publish a parameter group which contains only non-scenario parameters
        return;
    }

    {
        // For testing, dump parameter group to omc log
        ostringstream ss;
        ss << "parameter group: "
            << name << ":\n";
        for (auto sym : pp_symbol_list) {
            auto pgs = dynamic_cast<ParameterGroupSymbol*>(sym);
            if (pgs) {
                // element is a parameter group
                if (pgs->contains_scenario_parameter()) {
                    ss << "   group: "
                        << sym->name
                        << "\n";
                }
                else {
                    // do not publish a parameter group which contains only non-scenario parameters
                }
            }
            else {
                // element is a parameter
                auto param = dynamic_cast<ParameterSymbol*>(sym);
                if (param) {
                    if (param->source == ParameterSymbol::scenario_parameter) {
                        ss << "   param: "
                            << sym->name
                            << "\n";
                    }
                    else {
                        // do not publish non-scenario parameter
                    }
                }
                else {
                    // invalid parameter group member
                    // (previously detected error condition)
                }
            }
        }
        theLog->logMsg(ss.str().c_str());
    }
}

const bool ParameterGroupSymbol::contains_scenario_parameter()
{
    for (auto sym : pp_symbol_list) {
        auto pgs = dynamic_cast<ParameterGroupSymbol*>(sym);
        if (pgs) {
            // element is a parameter group
            // The following line contains a recursive call
            if (pgs->contains_scenario_parameter()) {
                return true;
            }
        }
        else {
            // element is a parameter
            auto param = dynamic_cast<ParameterSymbol*>(sym);
            if (param) {
                if (param->source == ParameterSymbol::scenario_parameter) {
                    return true;
                }
            }
            else {
                // invalid parameter group member
                // (previously detected error condition)
                return true;
            }
        }
    }
    return false;
}

