/**
* @file    AgentMemberSymbol.cpp
* Definitions for the AgentMemberSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include <algorithm>
#include <typeinfo>
#include <set>
#include "ast.h"
#include "parser_helper.h"

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

void AgentMemberSymbol::post_parse(int pass)
{
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse(pass);

    switch (pass) {
    case 1:
        // assign direct pointer to agent for use post-parse
        pp_agent = dynamic_cast<AgentSymbol *> (agent);
        break;
    default:
        break;
    }
}

void AgentDataMemberSymbol::post_parse(int pass)
{
    // First perform post-parse operations at next level up in the Symbol hierarchy
    super::post_parse(pass);

    // Now do operations specific to this level in the Symbol hierarchy
    switch (pass) {
    case 1:
        // Add this agent data symbol to the agent's list of all such symbols
        pp_agent->pp_agent_data_members.push_back(this);
        break;
    default:
        break;
    }
}

string AgentDataMemberSymbol::initial_value() const
{
    string result;
    switch (type) {

    case token::TK_bool:
        // for bool, default initial value is false
        result = token_to_string(token::TK_false);
        break;

    default:
        // for all other fundamental types, default initial value is 0
        result = "( " + token_to_string(type) + " ) 0";

    };
    return result;
}

CodeBlock AgentDataMemberSymbol::cxx_initialize_expression() const
{
    // example:              time.initialize(0);\n
    CodeBlock c;
    c += name + ".initialize( " + initial_value() + " );";
    return c;
}

CodeBlock AgentDataMemberSymbol::cxx_declaration_agent_scope()
{
    CodeBlock h;
    h += "";
    // example:         //* time */
    h += doxygen(name);
    return h;
}

CodeBlock AgentDataMemberSymbol::cxx_definition()
{
    CodeBlock c;
    c += "";
    // example:         //* time */
    c += doxygen(name);
    return c;
}



void AgentCallbackMemberSymbol::post_parse(int pass)
{
    // First perform post-parse operations at next level up in the Symbol hierarchy
    super::post_parse(pass);

    // Now do operations specific to this level in the Symbol hierarchy
    switch (pass) {
    case 1:
        // Nothing
        break;
    default:
        break;
    }
}

CodeBlock AgentCallbackMemberSymbol::cxx_declaration_agent_scope()
{
    // First get declaration code at next level up in the Symbol hierarchy
    CodeBlock h = super::cxx_declaration_agent_scope();

    // Now add declaration code specific to this level in the Symbol hierarchy

    return h;
}

CodeBlock AgentCallbackMemberSymbol::cxx_definition()
{
    // First get definition code at next level up in the Symbol hierarchy
    CodeBlock c = super::cxx_definition();

    // Now add definition code specific to this level in the Symbol hierarchy

    return c;
}


void AgentInternalSymbol::post_parse(int pass)
{
    // First perform post-parse operations at next level up in the Symbol hierarchy
    super::post_parse(pass);

    // Now do operations specific to this level in the Symbol hierarchy
    switch (pass) {
    case 1:
        // Nothing
        break;
    default:
        break;
    }
}

CodeBlock AgentInternalSymbol::cxx_initialize_expression() const
{
    CodeBlock c;
    // example:              om_in_DurationOfLife_alive = false;\n
    c += name + " = " + initial_value() + ";";
    return c;
}

CodeBlock AgentInternalSymbol::cxx_declaration_agent_scope()
{
    // First get declaration code at next level up in the Symbol hierarchy
    CodeBlock h = super::cxx_declaration_agent_scope();
    // Now add declaration code specific to this level in the Symbol hierarchy
    h += token_to_string(type) + " " + name + ";";
    return h;
}

CodeBlock AgentInternalSymbol::cxx_definition()
{
    // First get definition code at next level up in the Symbol hierarchy
    CodeBlock c = super::cxx_definition();
    return c;
}

void AgentEventTimeSymbol::post_parse(int pass)
{
    // First perform post-parse operations at next level up in the Symbol hierarchy
    super::post_parse(pass);

    // Now do operations specific to this level in the Symbol hierarchy
    switch (pass) {
    case 1:
        // assign direct pointer(s) for use post-parse
        pp_event = dynamic_cast<AgentEventSymbol *> (event);
        // Add this agent event time symbol to the agent's list of all such symbols
        pp_agent->pp_agent_event_times.push_back(this);
        break;
    case 2:
    {
              // E.g. Person
              string agent_name = pp_agent->name;
              // E.g. Person::timeMortalityEvent
              string time_func_name = pp_event->time_function->unique_name;
              // create sorted unduplicated list of identifiers in body of event time function
              set<string> identifiers;
              auto rng = memfunc_bodyids.equal_range(time_func_name);
              for_each(rng.first,
                  rng.second,
                  [&](unordered_multimap<string, string>::value_type& vt)
              {
                  identifiers.insert(vt.second);
              }
              );
              // iterate through list looking for agentvars with name = identifier
              for (auto identifier : identifiers) {
                  if (exists(identifier, pp_agent)) {
                      auto sym = get_symbol(identifier, pp_agent);
                      auto av = dynamic_cast<AgentVarSymbol *>(sym);
                      if (av) {
                          // dependency of time function on av detected
                          // E.g. om_time_StartPlayingEvent.make_dirty();
                          string line = name + ".make_dirty();";
                          av->pp_side_effects.push_back(line);
                      }
                  }
              }
              // add side-effect to time agentvar
              AgentVarSymbol *av = pp_agent->pp_time;
              // Eg. om_duration.wait( new_value - old_value );
              //string line = name + ".make_dirty();";
    }
        break;
    default:
        break;
    }
}

string AgentEventTimeSymbol::member_name(const Symbol *event)
{
    string result = "om_time_" + event->name;
    return result;
}

string AgentEventTimeSymbol::symbol_name(const Symbol *agent, const Symbol *event)
{
    string member = AgentEventTimeSymbol::member_name(event);
    string result = Symbol::symbol_name(member, agent);
    return result;
}

CodeBlock AgentEventTimeSymbol::cxx_declaration_agent_scope()
{
    // First get declaration code at next level up in the Symbol hierarchy
    CodeBlock h = super::cxx_declaration_agent_scope();
    // Now add declaration code specific to this level in the Symbol hierarchy
    //h += token_to_string(type) + " " + name + ";";
    string implement_func = pp_event->unique_name;
    string time_func = pp_event->time_function->unique_name;
    int event_id = pp_event->pp_event_id;
    h += "Event<" + agent->name + ", "
        + to_string(event_id) + ", "
        + "0, " // TODO event priority
        + "&" + implement_func + ", "
        + "&" + time_func + "> "
        + name
        + ";" "\n";
    return h;
}

CodeBlock AgentEventTimeSymbol::cxx_definition()
{
    // First get definition code at next level up in the Symbol hierarchy
    CodeBlock c = super::cxx_definition();
    return c;
}

/**
* Post-parse operations for AgentVarSymbol
*/

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

CodeBlock DurationAgentVarSymbol::cxx_declaration_agent_scope()
{
    // get declaration code common to all agentvars
    CodeBlock h = super::cxx_declaration_agent_scope();

    // add declaration code specific to derived agentvars

    // example:         DurationAgentVar<Time, Person, &duration_offset, &Person::duration_side_effects, nullptr> om_duration
    h += "DurationAgentVar<" + token_to_string(type) + ", "
        + agent->name + ", "
        + "&" + agent->name + "::" + side_effects_func() + ", "
        + "nullptr> "
        + name + ";" "\n";

    return h;
}

// static
Symbol * DurationAgentVarSymbol::get_symbol(const Symbol *agent)
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


/**
* Post-parse operations for DurationAgentVarSymbol
*/

void DurationAgentVarSymbol::post_parse(int pass)
{
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse(pass);

    switch (pass) {
    case 1:
        break;
    case 2:
    {
              // add side-effect to time agentvar
              AgentVarSymbol *av = pp_agent->pp_time;
              // Eg. om_duration.wait( new_value - old_value );
              string line = name + ".wait( new_value - old_value );";
              av->pp_side_effects.push_back(line);
    }
    default:
        break;
    }
}

// static
string ConditionedDurationAgentVarSymbol::member_name(const Symbol *observed, const Literal *constant)
{
    string result = "om_" + token_to_string(token::TK_duration) + "_" + observed->name + "_" + constant->cxx_token;
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
Symbol * ConditionedDurationAgentVarSymbol::get_symbol(const Symbol* agent, const Symbol* observed, const Literal* constant)
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
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse(pass);

    switch (pass) {
    case 1:
        // assign direct pointer for post-parse use
        pp_observed = dynamic_cast<AgentVarSymbol *> (observed);
        break;
    case 2:
    {
              // add side-effect to time agentvar
              AgentVarSymbol *av = pp_agent->pp_time;
              // Eg. om_duration.wait( new_value - old_value );
              string line = name + ".wait( new_value - old_value );";
              av->pp_side_effects.push_back(line);
    }
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

CodeBlock ConditionedDurationAgentVarSymbol::cxx_declaration_agent_scope()
{
    // get declaration code common to all agentvars
    CodeBlock h = super::cxx_declaration_agent_scope();

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

CodeBlock ConditionedDurationAgentVarSymbol::cxx_definition()
{
    // start with definition code from next higher level in symbol hierarchy
    CodeBlock c = super::cxx_definition();

    // add definition code specific to this kind of derived agentvar
    // example:        bool Person::om_duration_alive_true_condition()
    c += condition_decl_qualified();
    c += "{";
    // example:             return ( alive == true );
    c += "return (" + observed->name + " == " + constant->cxx_token + " );";
    c += "}";

    return c;
}

/**
* Post-parse operations for BuiltinAgentVarSymbol
*/

void BuiltinAgentVarSymbol::post_parse(int pass)
{
    // First perform post-parse operations at next level up in Symbol hierarchy
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

string SimpleAgentVarSymbol::initial_value() const
{
    string result;
    if (initializer != nullptr)
        result = initializer->cxx_token;
    else
        result = AgentVarSymbol::initial_value();

    return result;
}

void SimpleAgentVarSymbol::post_parse(int pass)
{
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse(pass);
}

CodeBlock SimpleAgentVarSymbol::cxx_declaration_agent_scope()
{
    // obtain declaration code common to all agentvars
    CodeBlock h = super::cxx_declaration_agent_scope();

    // add declaration code specific to simple agentvars

    // example:         SimpleAgentVar<bool, Person, &Person::alive_side_effects> alive;
    h += "SimpleAgentVar<" + token_to_string(type) + ", "
        + agent->name + ", "
        + "&" + agent->name + "::" + side_effects_func() + "> "
        + name + ";";

    return h;
}

void AgentFuncSymbol::post_parse(int pass)
{
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse(pass);

    switch (pass) {
    case 1:
        // Add this agentfunc to the agent's list of agentfuncs
        pp_agent->pp_agentfuncs.push_back(this);
        break;
    default:
        break;
    }
}



