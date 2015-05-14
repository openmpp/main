/**
* @file    TableMeasureSymbol.cpp
* Definitions for the TableMeasureSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "DerivedTableSymbol.h"
#include "TableMeasureSymbol.h"

using namespace std;

// static
string TableMeasureSymbol::symbol_name(const Symbol* derived_table, int index)
{
    assert(derived_table);
    return derived_table->name + ".Expr" + to_string(index);
}

string TableMeasureSymbol::pretty_name() const
{
    // example:     accumulator 0: sum(delta(interval(duration)))
    string result = " measure " + to_string(index) + ": " + measure_name;
    return result;
}

void TableMeasureSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // assign direct pointer to table for post-parse use
        pp_table = dynamic_cast<TableSymbol *> (pp_symbol(table));
        assert(pp_table); // parser guarantee

        // Add this measure to the table's list of measures
        pp_table->pp_measures.push_back(this);
        break;
    }
    default:
        break;
    }
}
