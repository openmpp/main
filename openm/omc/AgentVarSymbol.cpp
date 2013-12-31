/**
* @file    AgentVarSymbol.cpp
* Definitions for the AgentVarSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "AgentVarSymbol.h"
#include "AgentSymbol.h"
#include "CodeBlock.h"

using namespace std;

void AgentVarSymbol::post_parse(int pass)
{
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse(pass);

    switch (pass) {
    case 1:
        // Add this agentvar to the agent's list of all agentvars
        pp_agent->pp_agentvars.push_back(this);
        break;
    default:
        break;
    }
}

CodeBlock AgentVarSymbol::cxx_declaration_agent_scope()
{
    // First get declaration code at next level up in the Symbol hierarchy
    CodeBlock h = super::cxx_declaration_agent_scope();

    // Now add declaration code specific to this level in the Symbol hierarchy
    // example:        void time_side_effects(Time old_value, Time new_value);
    h += side_effects_decl() + ";";
    return h;
}

CodeBlock AgentVarSymbol::cxx_definition()
{
    // First get definition code at next level up in the Symbol hierarchy
    CodeBlock c = super::cxx_definition();

    // example:         Person::time_side_effects(Time old_value, Time new_value)
    c += side_effects_decl_qualified();
    c += "{";
    for (string line : pp_side_effects) {
        c += line;
    }
    c += "}";

    return c;
}

const string AgentVarSymbol::side_effects_func()
{
    return name + "_side_effects";
}

const string AgentVarSymbol::side_effects_decl()
{
    return "void " + side_effects_func() + "(" + token_to_string(type) + " old_value, " + token_to_string(type) + " new_value)";
}

const string AgentVarSymbol::side_effects_decl_qualified()
{
    return "void " + agent->name + "::" + side_effects_func() + "(" + token_to_string(type) + " old_value, " + token_to_string(type) + " new_value)";
}

