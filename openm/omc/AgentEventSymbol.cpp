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
    CodeBlock h;
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
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse(pass);

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


