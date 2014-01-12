/**
* @file    ConditionedDurationAgentVarSymbol.cpp
* Definitions for the ConditionedDurationAgentVarSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "ConditionedDurationAgentVarSymbol.h"
#include "AgentSymbol.h"
#include "AgentVarSymbol.h"
#include "BuiltinAgentVarSymbol.h"
#include "Literal.h"
#include "CodeBlock.h"

using namespace std;

// static
string ConditionedDurationAgentVarSymbol::member_name(const Symbol *observed, const Literal *constant)
{
    string result = "om_" + token_to_string(token::TK_duration) + "_" + observed->name + "_" + constant->value();
    return result;
}

// static
string ConditionedDurationAgentVarSymbol::symbol_name(const Symbol* agent, const Symbol* observed, const Literal* constant)
{
    string member = ConditionedDurationAgentVarSymbol::member_name(observed, constant);
    string result = Symbol::symbol_name(member, agent);
    return result;
}

// static
Symbol * ConditionedDurationAgentVarSymbol::create_symbol(const Symbol* agent, const Symbol* observed, const Literal* constant)
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
            pp_observed = dynamic_cast<AgentVarSymbol *> (observed);
            assert(pp_observed); // syntax error
    }
        break;
    case ePopulateDependencies:
        {
            // add side-effect to time agentvar
            AgentVarSymbol *av = pp_agent->pp_time;
            // Eg. om_duration.wait( new_value - old_value );
            string line = name + ".wait( new_value - old_value );";
            av->pp_side_effects.push_back(line);
        }
        break;
    default:
        break;
    }
}

const string ConditionedDurationAgentVarSymbol::condition_func()
{
    return name + "_condition";
}

const string ConditionedDurationAgentVarSymbol::condition_decl()
{
    return "bool " + condition_func() + "()";
}

const string ConditionedDurationAgentVarSymbol::condition_decl_qualified()
{
    return "bool " + agent->name + "::" + condition_func() + "()";
}

CodeBlock ConditionedDurationAgentVarSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    // add declaration code specific to this kind of derived agentvar
    // 
    // example:        bool om_duration_alive_true_condition();
    h += condition_decl() + ";";

    // example:         DurationAgentVar<Time, Person, &om_duration_alive_true_offset, &Person::om_duration_alive_true_side_effects, &Person::om_duration_alive_true_condition> om_duration
    h += "DurationAgentVar<" + token_to_string(type) + ", "
        + agent->name + ", "
        + "&" + agent->name + "::" + side_effects_func() + ", "
        + "&" + agent->name + "::" + condition_func() + "> "
        + name + ";";

    return h;
}

CodeBlock ConditionedDurationAgentVarSymbol::cxx_definition_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock c = super::cxx_definition_agent();

    // Perform operations specific to this level in the Symbol hierarchy.
    // add definition code specific to this kind of derived agentvar
    // example:        bool Person::om_duration_alive_true_condition()
    c += condition_decl_qualified();
    c += "{";
    // example:             return ( alive == true );
    c += "return (" + observed->name + " == " + constant->value() + " );";
    c += "}";

    return c;
}

