/**
* @file    DependencyGroupSymbol.cpp
* Definitions for the DependencyGroupSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "DependencyGroupSymbol.h"
#include "TableGroupSymbol.h"
#include "TableSymbol.h"

using namespace std;

void DependencyGroupSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // check that list is well-formed
        // and handle first element
        if (pp_symbol_list.size() < 2) {
            pp_error(LT("error : dependency list must have 2 or more symbols"));
        }
        bool first = true;
        for (auto sym : pp_symbol_list) {
            assert(sym);
            bool is_table = dynamic_cast<TableSymbol*>(sym);
            bool is_table_group = dynamic_cast<TableGroupSymbol*>(sym);
            if (!(is_table || is_table_group)) {
                pp_error(LT("error : invalid member '") + sym->name + LT("' of dependency list"));
            }
            if (first) {
                first = false;
                if (!is_table) {
                    pp_error(LT("error : invalid first member '") + sym->name + LT("' of dependency list"));
                }
                // the table requiring other tables (first in dependency statement)
                pp_table_requiring = dynamic_cast<TableSymbol*>(sym);
            }
        }
        // pop first element from symbol list
        // so that list contains just tables and table groups required by pp_table_requiring
        pp_symbol_list.pop_front();
        break;
    }
    case ePopulateCollections:
    {
        // add this to the complete list of dependency groups
        pp_all_dependency_groups.push_back(this);

        break;
    }
    case ePopulateDependencies:
    {
        // Add the expanded list of required tables to required set of the requiring table
        // and
        // Add the requiring table to the requiring set of each table in the expanded list of required tables.
        auto rtl = expanded_list();
        assert(pp_table_requiring);
        auto tbl_requiring = pp_table_requiring; // for clarity of names below
        for (auto sym : rtl) {
            auto tbl_required = dynamic_cast<TableSymbol*>(sym);
            assert(tbl_required);
            tbl_requiring->pp_tables_required.insert(tbl_required);
            tbl_required->pp_tables_requiring.insert(tbl_requiring);
        }

        break;
    }
    default:
        break;
    }
}

int DependencyGroupSymbol::counter = 0;
