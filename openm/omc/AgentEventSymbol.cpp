/**
* @file    AgentEventSymbol.cpp
* Definitions for the AgentEventSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "AgentEventSymbol.h"
#include "AgentSymbol.h"
#include "CodeBlock.h"

using namespace std;

/**
* Post-parse operations for AgentEventSymbol
*/

void AgentEventSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
        {
            // Add this agentevent to the agent's list of all agentevents
            pp_agent->pp_agentevents.push_back(this);
        }
        break;
    default:
        break;
    }
}

/**
* Post-parse operations for AgentMemberSymbol
*/


