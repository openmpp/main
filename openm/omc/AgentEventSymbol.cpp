/**
* @file    AgentEventSymbol.cpp
* Definitions for the AgentEventSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "AgentEventSymbol.h"
#include "AgentSymbol.h"
#include "CodeBlock.h"

using namespace std;

CodeBlock AgentEventSymbol::cxx_declaration_agent_scope()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent_scope();

    // Perform operations specific to this level in the Symbol hierarchy.
    h += "";
    // example:         //* Mortality */
    h += "//* " + name + " */";
    // example: void MortalityEvent(); 
    h += "void " + name + "();";
    // example: Time timeMortalityEvent(); 
    h += "Time " + time_function->name + "();";
    return h;
}

/**
* Post-parse operations for AgentEventSymbol
*/

void AgentEventSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case 1:
        // Add this agentevent to the agent's list of all agentevents
        pp_agent->pp_agentevents.push_back(this);
        break;
    default:
        break;
    }
}

/**
* Post-parse operations for AgentMemberSymbol
*/


