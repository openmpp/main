/**
* @file    EntityIncrementSymbol.cpp
* Definitions for the EntityIncrementSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include <string>
#include "EntityIncrementSymbol.h"
#include "AgentSymbol.h"
#include "AgentFuncSymbol.h"
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

CodeBlock EntityIncrementSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    h += "";
    h += "static const string om_name_" + name + ";";
    h += doxygen_short(pretty_name());
    h += "Increment<"
        + agent->name + ", "
        + "&om_name_" + name + ", "
        + "&" + table->init_increment_fn->unique_name + ", "
        + "&" + table->push_increment_fn->unique_name
        + ">";
    h += name + ";";
    return h;
}

CodeBlock EntityIncrementSymbol::cxx_definition_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    c += "const string " + pp_agent->name + "::om_name_" + name + " = \"" + pretty_name() + "\";";
 
    return c;
}

