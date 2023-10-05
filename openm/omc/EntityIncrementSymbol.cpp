/**
* @file    EntityIncrementSymbol.cpp
* Definitions for the EntityIncrementSymbol class.
*/
// Copyright (c) 2013-2022 OpenM++ Contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include <string>
#include "EntityIncrementSymbol.h"
#include "EntitySymbol.h"
#include "EntityFuncSymbol.h"
#include "CodeBlock.h"

using namespace std;

void EntityIncrementSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eAssignMembers:
    {
        break;
    }
    case ePopulateCollections:
    {
        break;
    }
    default:
        break;
    }
}

CodeBlock EntityIncrementSymbol::cxx_declaration_entity()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_entity();

    // get maximum index used in flattened table (including margin cells)
    auto max_index = table->cell_count() - 1;

    // determine smallest unsigned integer type capable of holding
    // the maximum index of this table
    string increment_storage_type;
    if (max_index <= std::numeric_limits<uint8_t>::max()) {
        increment_storage_type = "uint8_t";
    }
    else if (max_index <= std::numeric_limits<uint16_t>::max()) {
        increment_storage_type = "uint16_t";
    }
    else if (max_index <= std::numeric_limits<uint32_t>::max()) {
        increment_storage_type = "uint32_t";
    }
    else {
        increment_storage_type = "size_t";
    }

    // Perform operations specific to this level in the Symbol hierarchy.
    h += "";
    h += "static const std::string om_name_" + name + ";";
    h += doxygen_short(pretty_name());
    h += "Increment<"
        + entity->name + ", "
        + increment_storage_type + ", "
        + "&om_name_" + name + ", "
        + "&" + table->init_increment_fn->unique_name + ", "
        + "&" + table->push_increment_fn->unique_name
        + ">";
    h += name + ";";
    return h;
}

CodeBlock EntityIncrementSymbol::cxx_initialization_expression(bool type_default) const
{
    CodeBlock c;
    // increments do not participate in this initialization mechanism
    return c;
}

CodeBlock EntityIncrementSymbol::cxx_definition_entity()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_entity();

    // Perform operations specific to this level in the Symbol hierarchy.
    c += "const std::string " + pp_entity->name + "::om_name_" + name + " = \"" + pretty_name() + "\";";
 
    return c;
}

