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
    case eAssignMembers:
    {
        is_hidden = true;

        // assign global flags about what kinds of anon group statements are present in model code
        switch (anon_kind) {
        case eKind::parameters_suppress:
            Symbol::any_parameters_suppress = true;
            break;
        case eKind::parameters_retain:
            Symbol::any_parameters_retain = true;
            break;
        case eKind::tables_suppress:
            Symbol::any_tables_suppress = true;
            break;
        case eKind::tables_retain:
            Symbol::any_tables_retain = true;
            break;
        case eKind::parameters_to_tables:
            Symbol::any_parameters_to_tables = true;
            break;
        default:
            break;
        }
        break;
    }
    case eResolveDataTypes:
    {
        // issue error messages if model contains incompatible combinations of anon groups
        switch (anon_kind) {
        case eKind::hide:
            break;
        case eKind::parameters_suppress:
            if (any_parameters_retain) {
                pp_error(LT("error : a model cannot contain both parameters_suppress and parameters_retain statements"));
            }
            break;
        case eKind::parameters_retain:
            if (any_parameters_suppress) {
                pp_error(LT("error : a model cannot contain both parameters_suppress and parameters_retain statements"));
            }
            break;
        case eKind::tables_suppress:
            if (any_tables_retain) {
                pp_error(LT("error : a model cannot contain both tables_suppress and tables_retain statements"));
            }
            break;
        case eKind::tables_retain:
            if (any_tables_suppress) {
                pp_error(LT("error : a model cannot contain both tables_suppress and tables_retain statements"));
            }
            break;
        default:
            break;
        }
        break;
    }
    case ePopulateCollections:
    {
        // add this to the complete list of anon groups
        pp_all_anon_groups.push_back(this);

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
                    // just ignore 'hide' statement for tables,
                    // since in Modgen is just a hint to the UI and table may be required, eg for downstream model importing it.
                    //ts->is_internal = true;
                    continue;
                }
                pp_error(LT("error : '") + sym->name + LT("' in hide list is not a parameter or table"));
            }
            break;
        }
        case eKind::parameters_suppress:
        {
            for (auto sym : expanded_list()) {
                auto symbol_name = sym->name;
                auto ps = dynamic_cast<ParameterSymbol*>(sym);
                if (ps) {
                    if (ps->source == ParameterSymbol::parameter_source::scenario_parameter) {
                        // indicate that this scenario parameter is to be burned into the executable instead of published
                        ps->source = ParameterSymbol::parameter_source::fixed_parameter;
                    }
                }
                else {
                    pp_error(LT("error : '") + symbol_name + LT("' in parameters_suppress statement is not a parameter"));
                }
            }
            break;
        }
        case eKind::parameters_retain:
        {
            // Before this pass, all scenario parameters were changed to fixed parameters.
            // in preparation for this step, which switches retained parameters back to scenario parameters.
            for (auto sym : expanded_list()) {
                auto symbol_name = sym->name;
                auto ps = dynamic_cast<ParameterSymbol*>(sym);
                if (ps) {
                    if (ps->source == ParameterSymbol::parameter_source::fixed_parameter) {
                        // indicate that this parameter is to be a published scenario parameter, ie not burned into the executable.
                        ps->source = ParameterSymbol::parameter_source::scenario_parameter;
                    }
                }
                else {
                    pp_error(LT("error : '") + symbol_name + LT("' in parameters_retain statement is not a parameter"));
                }
            }
            break;
        }
        case eKind::tables_suppress:
        {
            for (auto sym : expanded_list()) {
                auto symbol_name = sym->name;
                auto ts = dynamic_cast<TableSymbol*>(sym);
                if (ts) {
                    // indicate that this table is to be suppressed from the model
                    ts->is_suppressed = true;
                }
                else {
                    pp_error(LT("error : '") + symbol_name + LT("' in tables_suppress statement is not a table"));
                }
            }
            break;
        }
        case eKind::tables_retain:
        {
            // Before this pass, is_suppressed was set to true for all non-internal tables
            // in preparation for this step, which switches retained tables back to false.
            for (auto sym : expanded_list()) {
                auto symbol_name = sym->name;
                auto ts = dynamic_cast<TableSymbol*>(sym);
                if (ts) {
                    // indicate that this table is not to be suppressed.
                    ts->is_suppressed = false;
                }
                else {
                    pp_error(LT("error : '") + symbol_name + LT("' in tables_retain statement is not a table"));
                }
            }
            break;
        }
        case eKind::parameters_to_tables:
        {
            for (auto sym : expanded_list()) {
                auto symbol_name = sym->name;
                auto ps = dynamic_cast<ParameterSymbol*>(sym);
                if (ps) {
                    // indicate that this parameter, if derived, is also to be published as a table.
                    if (ps->source == ParameterSymbol::parameter_source::derived_parameter) {
                        ps->publish_as_table = true;
                    }
                }
                else {
                    pp_error(LT("error : '") + symbol_name + LT("' in parameters_to_tables statement is not a parameter"));
                }
            }
            pp_warning(LT("warning : parameters_to_tables is not implemented"));
            break;
        }
        default:
            assert(false); // logic guarantee
        } // switch (anon_kind)
    } // case ePopulateCollections:
    default:
        break;
    }
}

int AnonGroupSymbol::counter = 0;
