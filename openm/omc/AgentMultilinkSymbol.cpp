/**
* @file    AgentMultilinkSymbol.cpp
* Definitions for the AgentMultilinkSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "AgentMultilinkSymbol.h"
#include "LinkAgentVarSymbol.h"
#include "AgentSymbol.h"
#include "TypeSymbol.h"
#include "CodeBlock.h"

using namespace std;

void AgentMultilinkSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
        {
            // Add this multilink agentvar symbol to the agent's list of all such symbols
            pp_agent->pp_multilink_members.push_back(this);
        }
        break;
    case ePopulateDependencies:
        {
            // Dependency on agentvars in expression
            //CodeBlock& c = side_effects_fn->func_body;
            //auto other = reciprocal_link;

            // The link is multi

            // Multi-links cannot be changed directly by assignment.
            // Instead agents are added or removed from the multi-link using member functions.
            // Those member functions take care of maintaining the reciprocal link.
        }
        break;
    default:
        break;
    }
}

CodeBlock AgentMultilinkSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.

    //h += "MultiLinkAgentVar<" + pp_data_type->name + ", "
    //    + agent->name + ", "
    //    + reciprocal_link->agent->name + ", "
    //    + "&" + "TODO" + "> ";
    //h += name + ";";

    return h;
}



