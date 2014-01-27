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
        pp_time_func = dynamic_cast<AgentFuncSymbol *> (pp_symbol(time_func));
        assert(pp_time_func); // parser guarantee

        pp_implement_func = dynamic_cast<AgentFuncSymbol *> (pp_symbol(implement_func));
        assert(pp_implement_func); // parser guarantee

        // Add this agent event time symbol to the agent's list of all such symbols
        pp_agent->pp_agent_events.push_back(this);
    }
    break;
    case ePopulateDependencies:
    {
        // E.g. Person
        string agent_name = pp_agent->name;
        // E.g. Person::timeMortalityEvent
        string time_func_name = pp_time_func->unique_name;
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

CodeBlock AgentEventSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.

    string implement_func = pp_implement_func->unique_name;
    string time_func = pp_time_func->unique_name;
    int event_id = pp_event_id;
    h += "";
    h += doxygen_short("Event: " + implement_func);
    // example: void MortalityEvent(); 
    h += "void " + pp_implement_func->name + "();";
    // example: Time timeMortalityEvent(); 
    h += "Time " + pp_time_func->name + "();";
    h += "Event<" + agent->name + ", "
        + to_string(event_id) + ", "
        + "0, " // TODO event priority
        + "&" + implement_func + ", "
        + "&" + time_func + "> "
        + name
        + ";" "\n";

    return h;
}

