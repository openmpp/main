/**
* @file    ConditionedDurationAgentVarSymbol.cpp
* Definitions for the ConditionedDurationAgentVarSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "ConditionedDurationAgentVarSymbol.h"
#include "AgentSymbol.h"
#include "AgentVarSymbol.h"
#include "AgentFuncSymbol.h"
#include "BuiltinAgentVarSymbol.h"
#include "ConstantSymbol.h"
#include "Literal.h"
#include "CodeBlock.h"

using namespace std;

// static
string ConditionedDurationAgentVarSymbol::member_name(const Symbol *observed, const ConstantSymbol *constant)
{
    string result = "om_" + token_to_string(token::TK_duration) + "_" + observed->name + "_" + constant->value();
    return result;
}

// static
string ConditionedDurationAgentVarSymbol::symbol_name(const Symbol* agent, const Symbol* observed, const ConstantSymbol* constant)
{
    string member = ConditionedDurationAgentVarSymbol::member_name(observed, constant);
    string result = Symbol::symbol_name(member, agent);
    return result;
}

// static
Symbol * ConditionedDurationAgentVarSymbol::create_symbol(const Symbol* agent, const Symbol* observed, const ConstantSymbol* constant)
{
    Symbol *sym = nullptr;
    string nm = ConditionedDurationAgentVarSymbol::symbol_name(agent, observed, constant);
    auto it = symbols.find(nm);
    if (it != symbols.end())
        sym = it->second;
    else
        sym = new ConditionedDurationAgentVarSymbol(agent, observed, constant);

    return sym;
}


/**
* Post-parse operations for ConditionedDurationAgentVarSymbol
*/

void ConditionedDurationAgentVarSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
        {
            // assign direct pointer for post-parse use
            pp_observed = dynamic_cast<AgentVarSymbol *> (pp_symbol(observed));
            assert(pp_observed); // syntax error
        }
        break;
    case ePopulateDependencies:
        {
            // add side-effect to time agentvar
            AgentVarSymbol *av = pp_agent->pp_time;
            CodeBlock& c = av->side_effects_fn->func_body;
            c += "// Advance time for " + pretty_name();
            // Eg. om_duration.advance( new_value - old_value );
            string line = name + ".advance( new_value - old_value );";
            c += line;
            c += "";
        }
        break;
    default:
        break;
    }
}

CodeBlock ConditionedDurationAgentVarSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.

    // example:         DurationAgentVar<Time, Person, &om_duration_alive_true_offset, &Person::om_duration_alive_true_side_effects, &Person::om_duration_alive_true_condition> om_duration
    h += "DurationAgentVar<"
        + pp_data_type->name + ", "
        + pp_data_type->wrapped_type() + ", "
        + agent->name + ", "
        + "&" + side_effects_fn->unique_name + ", "
        + "&" + condition_fn->unique_name + ">";
    h += name + ";";

    return h;
}


