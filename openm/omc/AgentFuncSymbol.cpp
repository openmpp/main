/**
 * @file    AgentFuncSymbol.cpp
 * Definitions for the AgentFuncSymbol class.
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "AgentFuncSymbol.h"
#include "AgentSymbol.h"

using namespace std;

void AgentFuncSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
        // Add this agentfunc to the agent's list of agentfuncs
        pp_agent->pp_agentfuncs.push_back(this);
        break;
    default:
        break;
    }
}


