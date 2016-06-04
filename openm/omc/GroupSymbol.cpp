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
    case eResolveDataTypes:
    {
        // verify non-circularity
        if (is_circular()) {
            pp_error("error : group '" + name + "' contains circular reference");
        }
        break;
    }
    default:
        break;
    }
}


bool GroupSymbol::is_circular() const
{
    //TODO implement circular check of groups in pp_symbol_list
    // Recurse through list, building set of group symbol names.
    // If hit a name already encountered, return true.
    // otherwise return false.
    return false;
}


list<Symbol *> GroupSymbol::expanded_list() const
{
    list<Symbol *> exp_list = pp_symbol_list;

    // if circular, return an empty list
    if (!is_circular()) {
        //TODO - expand the list, recursively
    }

    return exp_list;
}

