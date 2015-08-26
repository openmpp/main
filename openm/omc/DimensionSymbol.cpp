/**
* @file    DimensionSymbol.h
* Definitions for the DimensionSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "DimensionSymbol.h"
#include "AgentVarSymbol.h"
#include "EnumerationSymbol.h"

// static
string DimensionSymbol::symbol_name(const Symbol* symbol_with_dimensions, int index, bool after_analysis_dim)
{
    assert(symbol_with_dimensions);
    // For entity tables, the numeric identifier is based on all dimensions, including the analysis dimension
    int numeric_id = index + (after_analysis_dim ? 1 : 0);
    return symbol_with_dimensions->name + ".Dim" + to_string(numeric_id);
}

void DimensionSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        if (attribute) {
            // Attribute was assigned during parsing.
            // The dimension symbol is in an entity table or entity set.
            // assign direct pointer to attribute for post-parse use
            pp_attribute = dynamic_cast<AgentVarSymbol *> (pp_symbol(attribute));
            if (!pp_attribute) {
                pp_error("'" + (*attribute)->name + "' must be an attribute to be used as a dimension");
            }
        }
        else {
            assert(enumeration); // grammar guarnatee
            // Attribute was not assigned during parsing.
            // The dimension symbol is in a derived table.
            // There is no attribute associated with this dimension.
            // An enumeration was specified for this dimension.
            pp_enumeration = dynamic_cast<EnumerationSymbol *> (pp_symbol(enumeration));
            if (!pp_enumeration) {
                pp_error("'" + (*enumeration)->name + "' must be an enumeration to be used as a dimension");
            }
        }
        break;
    }
    case ePopulateCollections:
    {
        if (pp_attribute) {
            // Assign direct pointer to enumeration for post-parse use
            pp_enumeration = dynamic_cast<EnumerationSymbol *>(pp_attribute->pp_data_type);
            if (!pp_enumeration) {
                pp_error("The datatype of '" + pp_attribute->name + "' must be an enumeration to be used as a dimension");
            }
        }
        break;
    }
    default:
        break;
    }
}

