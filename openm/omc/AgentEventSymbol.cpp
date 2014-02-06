/**
* @file    AgentEventSymbol.cpp
* Definitions for the AgentEventSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

//#include <cassert>
//#include <algorithm>
//#include <typeinfo>
#include <cassert>
#include <set>
#include "AgentEventSymbol.h"
#include "AgentSymbol.h"
#include "AgentVarSymbol.h"
#include "BuiltinAgentVarSymbol.h"
#include "AgentFuncSymbol.h"
#include "CodeBlock.h"

using namespace std;

void AgentEventSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
    {
        // Add this agent event time symbol to the agent's list of all such symbols
        pp_agent->pp_agent_events.push_back(this);
    }
    break;
    case ePopulateDependencies:
    {
        // E.g. Person
        string agent_name = pp_agent->name;
        // E.g. Person::timeMortalityEvent
        string time_func_name = time_func->unique_name;
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
                    av->side_effects_fn->func_body += line;
                }
            }
        }
        // add side-effect to time agentvar
        AgentVarSymbol *av = pp_agent->pp_time;
        // Eg. om_duration.advance( new_value - old_value );
        //string line = name + ".make_dirty();";
    }
    break;
    default:
    break;
    }
}

CodeBlock AgentEventSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.

    int event_id = pp_event_id;
    h += "";
    h += "Event<" + agent->name + ", "
        + to_string(event_id) + ", "
        + "0, " // TODO event priority
        + "&" + implement_func->unique_name + ", "
        + "&" + time_func->unique_name + "> "
        + name
        + ";" "\n";

    return h;
}

