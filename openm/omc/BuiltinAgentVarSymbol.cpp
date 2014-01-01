/**
 * @file    BuiltinAgentVarSymbol.cpp
 * Definitions for the BuiltinAgentVarSymbol class.
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "BuiltinAgentVarSymbol.h"
#include "AgentSymbol.h"
#include "CodeBlock.h"

using namespace std;

void BuiltinAgentVarSymbol::post_parse(int pass)
{
    // Hook into post_parse hierarchical calling chain
    super::post_parse(pass);

    switch (pass) {
    case 1:
        break;
    case 2:
        if (name == "age") {
            // add side-effect to time agentvar
            AgentVarSymbol *av = pp_agent->pp_time;
            string line = "age.set( age.get() + new_value - old_value );";
            av->pp_side_effects.push_back(line);
        }
    default:
        break;
    }
}

CodeBlock BuiltinAgentVarSymbol::cxx_declaration_agent_scope()
{
    // get declaration code common to all agentvars
    CodeBlock h = super::cxx_declaration_agent_scope();

    // example:         AgentVar<Time, Person, &Person::time_side_effects> time;
    h += "AgentVar<" + token_to_string(type) + ", "
        + agent->name + ", "
        + "&" + agent->name + "::" + side_effects_func() + "> "
        + name + ";";

    return h;
}


