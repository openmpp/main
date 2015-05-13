/**
* @file    DerivedTablePlaceholderSymbol.cpp
* Definitions for the DerivedTablePlaceholderSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "DerivedTableSymbol.h"
#include "DerivedTablePlaceholderSymbol.h"

using namespace std;

// static
string DerivedTablePlaceholderSymbol::symbol_name(const Symbol* derived_table, int index)
{
    assert(derived_table);
    return derived_table->name + ".Expr" + to_string(index);
}

string DerivedTablePlaceholderSymbol::pretty_name() const
{
    // example:     accumulator 0: sum(delta(interval(duration)))
    string result = " placeholder " + to_string(index) + ": " + placeholder_name;
    return result;
}

void DerivedTablePlaceholderSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // assign direct pointer to derived table for post-parse use
        pp_derived_table = dynamic_cast<DerivedTableSymbol *> (pp_symbol(derived_table));
        assert(pp_derived_table); // parser guarantee

        // Add this placeholder to the derived table's list of placeholders
        pp_derived_table->pp_placeholders.push_back(this);
        break;
    }
    default:
        break;
    }
}
