/**
* @file    TableDimensionSymbol.h
* Definitions for the TableDimensionSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "TableDimensionSymbol.h"
#include "TableSymbol.h"

// static
string TableDimensionSymbol::symbol_name(const Symbol* table, int index)
{
    assert(table);
    return table->name + ".Dim" + to_string(index);
}

void TableDimensionSymbol::post_parse(int pass)
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

        // Add this table dimension to the table's list of dimensions
        //pp_table->pp_dimensions.push_back(this);
        break;
    }
    default:
        break;
    }
}

