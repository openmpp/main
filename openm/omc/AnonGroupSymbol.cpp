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
        // assign global flags about what kinds of anon group statements are present in model code
        switch (anon_kind) {
        case eKind::hide:
            Symbol::any_hide = true;
            break;
        case eKind::show:
            Symbol::any_show = true;
            break;
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
            if (any_show) {
                pp_error(LT("error : a model cannot contain both hide and show statements"));
            }
            break;
        case eKind::show:
            if (any_hide) {
                pp_error(LT("error : a model cannot contain both hide and show statements"));
            }
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
            // Iterate expanded list and raise error if invalid
            for (auto sym : expanded_list()) {
                auto symbol_name = sym->name;
                auto ps = dynamic_cast<ParameterSymbol*>(sym);
                if (ps) {
                    continue;
                }
                auto ts = dynamic_cast<TableSymbol*>(sym);
                if (ts) {
                    continue;
                }
                pp_error(LT("error : '") + symbol_name + LT("' in hide list is not a parameter or table"));
            }
            // Iterate non-expanded list and mark each element hidden
            for (auto sym : pp_symbol_list) {
                auto symbol_name = sym->name;
                {
                    auto ps = dynamic_cast<ParameterSymbol*>(sym);
                    if (ps) {
                        ps->is_hidden = true;
                        continue;
                    }
                }
                {
                    auto ts = dynamic_cast<TableSymbol*>(sym);
                    if (ts) {
                        ts->is_hidden = true;
                        continue;
                    }
                }
                {
                    auto gs = dynamic_cast<GroupSymbol*>(sym);
                    if (gs) {
                        gs->is_hidden = true;
                        continue;
                    }
                }
                pp_error(LT("error : '") + symbol_name + LT("' in show list is not a parameter, table, or group"));
            }
            break;
        }
        case eKind::show:
        {
            // Iterate expanded list and set all parameters and tables to non-hidden
            // Note that true argument to expanded_list includes group symbols int he expansion
            for (auto sym : expanded_list(true)) {
                auto symbol_name = sym->name;
                {
                    auto ps = dynamic_cast<ParameterSymbol*>(sym);
                    if (ps) {
                        ps->is_hidden = false;
                        continue;
                    }
                }
                {
                    auto ts = dynamic_cast<TableSymbol*>(sym);
                    if (ts) {
                        ts->is_hidden = false;
                        continue;
                    }
                }
                {
                    auto gs = dynamic_cast<GroupSymbol*>(sym);
                    if (gs) {
                        gs->is_hidden = false;
                        continue;
                    }
                }
                pp_error(LT("error : '") + symbol_name + LT("' in show list is not a parameter, table, or group"));
            }
            break;
        }
        case eKind::parameters_suppress:
        {
            for (auto sym : expanded_list()) {
                auto symbol_name = sym->name;
                auto ps = dynamic_cast<ParameterSymbol*>(sym);
                if (!ps) {
                    pp_error(LT("error : '") + symbol_name + LT("' in parameters_suppress statement is not a parameter"));
                }
            }
            break;
        }
        case eKind::parameters_retain:
        {
            for (auto sym : expanded_list()) {
                auto symbol_name = sym->name;
                auto ps = dynamic_cast<ParameterSymbol*>(sym);
                if (!ps) {
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
                auto ps = dynamic_cast<ParameterSymbol*>(sym);
                if (ps && ps->is_derived()) {
                    // indicate that this derived parameter is to be suppressed if marked for export as table
                    ps->is_suppressed = true;
                }
                else if (ts) {
                    // indicate that this table is to be suppressed from the model
                    ts->is_suppressed = true;
                }
                else {
                    pp_error(LT("error : '") + symbol_name + LT("' in tables_suppress statement is not a table or derived parameter"));
                }
            }
            break;
        }
        case eKind::tables_retain:
        {
            // Before this pass, is_suppressed was set to true for all non-internal tables and derived parameters
            // in preparation for this step, which switches retained tables back to false.
            for (auto sym : expanded_list()) {
                auto symbol_name = sym->name;
                auto ts = dynamic_cast<TableSymbol*>(sym);
                auto ps = dynamic_cast<ParameterSymbol*>(sym);
                if (ps && ps->is_derived()) {
                    // indicate that this derived parameter is not to be suppressed if marked for export as table
                    ps->is_suppressed = false;
                }
                else if (ts) {
                    // indicate that this table is not to be suppressed.
                    ts->is_suppressed = false;
                }
                else {
                    pp_error(LT("error : '") + symbol_name + LT("' in tables_retain statement is not a table or derived parameter"));
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
                    if (ps->is_derived()) {
                        ps->publish_as_table = true;
                    }
                    else {
                        pp_error(LT("error : '") + symbol_name + LT("' in parameters_to_tables is not a derived parameter"));
                    }
                }
                else {
                    pp_error(LT("error : '") + symbol_name + LT("' in parameters_to_tables is not a parameter"));
                }
            }
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
