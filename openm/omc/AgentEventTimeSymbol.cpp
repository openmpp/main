/**
* @file    AgentEventTimeSymbol.cpp
* Definitions for the AgentEventTimeSymbol class.
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
