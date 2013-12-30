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


