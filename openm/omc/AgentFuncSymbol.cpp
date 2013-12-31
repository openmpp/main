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
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse(pass);

    switch (pass) {
    case 1:
        // Add this agentfunc to the agent's list of agentfuncs
        pp_agent->pp_agentfuncs.push_back(this);
        break;
    default:
        break;
    }
}


