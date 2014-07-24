/**
* @file    EntitySetSymbol.cpp
* Definitions for the EntitySetSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "EntitySetSymbol.h"
#include "AgentSymbol.h"
#include "CodeBlock.h"

using namespace std;

void EntitySetSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
        {
            // assign direct pointer to agent for use post-parse
            pp_agent = dynamic_cast<AgentSymbol *> (pp_symbol(agent));
            assert(pp_agent); // parser guarantee

            // add this entity set to the complete list of entity sets
            pp_all_entity_sets.push_back(this);

            // Add this entity set to the agent's list of entity sets
            pp_agent->pp_agent_entity_sets.push_back(this);
        }
        break;
    default:
        break;
    }
}

CodeBlock EntitySetSymbol::cxx_declaration_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    h += "extern EntitySet<" + pp_agent->name + "> "+ name + ";";
    h += "";

    return h;
}

CodeBlock EntitySetSymbol::cxx_definition_global()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_global();

    // Perform operations specific to this level in the Symbol hierarchy.
    c += "EntitySet<" + pp_agent->name + "> "+ name + ";";
    c += "";

    return c;
}