/**
* @file    AgentSymbol.cpp
* Implements the AgentSymbol derived class of the Symbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "AgentSymbol.h"
#include "BuiltinAgentVarSymbol.h"
#include "AgentInternalSymbol.h"
#include "NumericSymbol.h"

using namespace std;

void AgentSymbol::create_builtin_symbols()
{
    // Create builtin agentvars for this agent: time, age, events, agent_id
    if (!exists("time", this))
        new BuiltinAgentVarSymbol("time", this, NumericSymbol::get_typedef_symbol(token::TK_Time));
    if (!exists("age", this))
        new BuiltinAgentVarSymbol("age", this, NumericSymbol::get_typedef_symbol(token::TK_Time));
    if (!exists("events", this))
        new BuiltinAgentVarSymbol("events", this, NumericSymbol::get_typedef_symbol(token::TK_counter));
    if (!exists("agent_id", this))
        new BuiltinAgentVarSymbol("agent_id", this, NumericSymbol::get_typedef_symbol(token::TK_int));

    // TODO: Remove test - Create internal data members for this agent: allow_assignment
    if (!exists("allow_assignment", this))
        new AgentInternalSymbol("allow_assignment", this, Symbol::get_symbol(Symbol::token_to_string(token::TK_bool)));
}

void AgentSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
        {
            // Add this agent to the complete list of agents.
            pp_all_agents.push_back(this);

            // assign direct pointer to builtin member 'time' for use post-parse
            pp_time = dynamic_cast<BuiltinAgentVarSymbol *>(get_symbol("time", this));
            assert(pp_time); // parser guarantee
        }
        break;
    default:
        break;
    }
}


