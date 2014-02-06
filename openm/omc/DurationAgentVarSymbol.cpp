/**
* @file    DurationAgentVarSymbol.cpp
* Definitions for the DurationAgentVarSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "DurationAgentVarSymbol.h"
#include "AgentSymbol.h"
#include "AgentVarSymbol.h"
#include "AgentFuncSymbol.h"
#include "BuiltinAgentVarSymbol.h"
#include "CodeBlock.h"

using namespace std;

// static
string DurationAgentVarSymbol::member_name()
{
    string result = "om_" + token_to_string(token::TK_duration);
    return result;
}

// static
string DurationAgentVarSymbol::symbol_name(const Symbol *agent)
{
    string member = DurationAgentVarSymbol::member_name();
    string result = Symbol::symbol_name(member, agent);
    return result;
}

CodeBlock DurationAgentVarSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    // add declaration code specific to derived agentvars

    // example:         DurationAgentVar<Time, Person, &duration_offset, &Person::duration_side_effects, nullptr> om_duration
    h += "DurationAgentVar<" + pp_data_type->name + ", "
        + agent->name + ", "
        + "&" + side_effects_fn->unique_name + ", "
        + "nullptr> "
        + name + ";" "\n";

    return h;
}

// static
Symbol * DurationAgentVarSymbol::create_symbol(const Symbol *agent)
{
    Symbol *sym = nullptr;
    string nm = DurationAgentVarSymbol::symbol_name(agent);
    auto it = symbols.find(nm);
    if (it != symbols.end())
        sym = it->second;
    else
        sym = new DurationAgentVarSymbol(agent);

    return sym;
}


void DurationAgentVarSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateDependencies:
        {
            // add side-effect to time agentvar
            AgentVarSymbol *av = pp_agent->pp_time;
            // Eg. om_duration.advance( new_value - old_value );
            string line = name + ".advance( new_value - old_value );";
            av->pp_side_effects.push_back(line);
            av->side_effects_fn->func_body += line;
        }
        break;
    default:
        break;
    }
}


