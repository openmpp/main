/**
* @file    DerivedTableDimensionSymbol.h
* Definitions for the DerivedTableDimensionSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "DerivedTableDimensionSymbol.h"
#include "EnumerationSymbol.h"

// static
string DerivedTableDimensionSymbol::symbol_name(const Symbol* derived_table, int index, bool after_analysis_dim)
{
    assert(derived_table);
    // The numeric identifier is based on all dimensions, including the analysis dimension
    int numeric_id = index + (after_analysis_dim ? 1 : 0);
    return derived_table->name + ".Dim" + to_string(numeric_id);
}

void DerivedTableDimensionSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // assign direct pointer to enumeration for post-parse use
        pp_enumeration = dynamic_cast<EnumerationSymbol *> (pp_symbol(enumeration));
        if (!pp_enumeration) {
            pp_error("'" + enumeration->name + "' must be an enumeration to be used as a dimension");
        }
        break;
    }
    default:
        break;
    }
}

