/**
* @file    GroupSymbol.cpp
* Definitions for the GroupSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "GroupSymbol.h"

using namespace std;

void GroupSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // create post-parse collection of groups and symbols
        for (auto sym : symbol_list) {
            auto pp_sym = pp_symbol(sym);
            assert(pp_sym);
            pp_symbol_list.push_back(pp_sym);
        }
        break;
    }
    case ePopulateCollections:
    {
        // verify non-circularity
        if (is_circular()) {
            pp_error(LT("error : circular reference in group '") + name + LT("'"));
        }
        break;
    }
    default:
        break;
    }
}

bool GroupSymbol::is_circular() const
{
    return is_circular_helper(name);
}


bool GroupSymbol::is_circular_helper(const string & name) const
{
    for (auto sym : pp_symbol_list) {
        auto gs = dynamic_cast<GroupSymbol *>(sym);
        if (gs) {
            if (name == gs->name) {
                return true;
            }
            else {
                if (gs->is_circular_helper(name)) {
                    return true;
                }
            }
        }
    }
    return false;
}


list<Symbol *> GroupSymbol::expanded_list() const
{
    list<Symbol *> result = pp_symbol_list;

    // Protect from recursive stak overflow
    // by performing circularity check.
    // If circular, return an empty list.
    if (!is_circular()) {
        // expand the list, recursively
        result = expand_group();
    }

    return result;
}

list<Symbol *> GroupSymbol::expand_group() const
{
    list<Symbol *> result;

    for (auto sym : pp_symbol_list) {
        auto gs = dynamic_cast<GroupSymbol *>(sym);
        if (gs) {
            result.splice(result.end(), gs->expand_group());
        }
        else {
            result.push_back(sym);
        }
    }

    return result;
}
