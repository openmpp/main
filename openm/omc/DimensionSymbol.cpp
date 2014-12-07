/**
* @file    DimensionSymbol.h
* Definitions for the DimensionSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "DimensionSymbol.h"
#include "AgentVarSymbol.h"
#include "EnumerationSymbol.h"

// static
string DimensionSymbol::symbol_name(const Symbol* table_or_entity_set, int index)
{
    assert(table_or_entity_set);
    return table_or_entity_set->name + ".Dim" + to_string(index);
}

void DimensionSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        // assign direct pointer to attribute for post-parse use
        pp_attribute = dynamic_cast<AgentVarSymbol *> (pp_symbol(attribute));
        if (!pp_attribute) {
            pp_error("'" + attribute->name + "' must be an attribute to be used as a dimension");
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

