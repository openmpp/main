/**
* @file    DimensionSymbol.h
* Definitions for the DimensionSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "DimensionSymbol.h"
#include "TableSymbol.h"
#include "AgentVarSymbol.h"

// static
string DimensionSymbol::symbol_name(const Symbol* table, int index)
{
    assert(table);
    return table->name + ".Dim" + to_string(index);
}

void DimensionSymbol::post_parse(int pass)
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

        // assign direct pointer to attribute for post-parse use
        pp_attribute = dynamic_cast<AgentVarSymbol *> (pp_symbol(attribute));
        assert(pp_attribute); // parser guarantee

        // Add this table dimension to the table's list of dimensions
        //pp_table->pp_dimensions.push_back(this);
        break;
    }
    default:
        break;
    }
}

