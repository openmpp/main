/**
* @file    AgentSymbol.cpp
* Implements the AgentSymbol derived class of the Symbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include <algorithm>
#include <typeinfo>
#include <set>
#include "ast.h"
#include "token.h"

using namespace std;

void AgentSymbol::create_builtin_symbols()
{
    //Create builtin agentvars for this agent: time, age, events, agent_id
    if (!exists("time", this))
        new BuiltinAgentVarSymbol("time", this, token::TK_Time);
    if (!exists("age", this))
        new BuiltinAgentVarSymbol("age", this, token::TK_Time);
    if (!exists("events", this))
        new BuiltinAgentVarSymbol("events", this, token::TK_counter);
    if (!exists("agent_id", this))
        new BuiltinAgentVarSymbol("agent_id", this, token::TK_int);

    // TODO: Remove test - Create internal data members for this agent: allow_assignment
    if (!exists("allow_assignment", this))
        new AgentInternalSymbol("allow_assignment", this, token::TK_bool);
}

void AgentSymbol::post_parse(int pass)
{
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse(pass);

    switch (pass) {
    case 1:
        // Add this agent to the complete list of agents.
        pp_agents.push_back(this);
        break;
    default:
        break;
    }

    // assign pointers to builtin members for use post-parse
    pp_time = dynamic_cast<BuiltinAgentVarSymbol *>(get_symbol("time", this));
    pp_age = dynamic_cast<BuiltinAgentVarSymbol *>(get_symbol("age", this));
    pp_events = dynamic_cast<BuiltinAgentVarSymbol *>(get_symbol("events", this));
}


