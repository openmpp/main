/**
* @file    AnonGroupSymbol.cpp
* Definitions for the AnonGroupSymbol class.
*/
// Copyright (c) 2013-2021 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "AnonGroupSymbol.h"
#include "ParameterSymbol.h"
#include "TableSymbol.h"

using namespace std;

void AnonGroupSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
    {
        // add this to the complete list of anon groups
        is_hidden = true; // SFG not sure what purpose this serves, at the group level...
        pp_all_anon_groups.push_back(this);
        break;
    }
    case ePopulateDependencies:
    {
        switch (anon_kind) {
        case eKind::hide:
        {
            // Iterate expanded list and set is_hidden for parameters and is_internal for tables
            for (auto sym : expanded_list()) {
                auto ps = dynamic_cast<ParameterSymbol*>(sym);
                if (ps) {
                    ps->is_hidden = true;
                    continue;
                }
                auto ts = dynamic_cast<TableSymbol*>(sym);
                if (ts) {
                    ts->is_internal = true;
                    continue;
                }
                pp_error(LT("error : '") + sym->name + LT("' in hide list is not a parameter or table"));
            }
            break;
        }
        case eKind::parameters_suppress:
        {
            for (auto sym : expanded_list()) {
                auto ps = dynamic_cast<ParameterSymbol*>(sym);
                if (ps) {
                    // indicate that this parameter is to be burned into executable instead of published
                    ps->source = ParameterSymbol::parameter_source::fixed_parameter;
                }
                else {
                    pp_error(LT("error : '") + sym->name + LT("' in parameters_suppress statement is not a parameter"));
                }
            }
            break;
        }
        case eKind::parameters_retain:
        {
            // Before this pass, code in Symbol::post_parse_all() changed all Scenario parameters to Fixed
            // in preparation for this step, which switches selected parameters back.
            for (auto sym : expanded_list()) {
                auto ps = dynamic_cast<ParameterSymbol*>(sym);
                if (ps) {
                    // indicate that this parameter is to be a published scenario parameter, ie not burned into the executable.
                    ps->source = ParameterSymbol::parameter_source::scenario_parameter;
                }
                else {
                    pp_error(LT("error : '") + sym->name + LT("' in parameters_retain statement is not a parameter"));
                }
            }
            break;
        }
        case eKind::tables_suppress:
        case eKind::tables_retain:
        case eKind::parameters_to_tables:
        {
            pp_error(LT("error : not implemented"));
            break;
        }
        default:
            assert(false); // logic guarantee
        } // switch (anon_kind)
    } // case ePopulateDependencies:
    default:
        break;
    }
}

int AnonGroupSymbol::counter = 0;