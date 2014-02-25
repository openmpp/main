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

void AgentEventSymbol::create_auxiliary_symbols(Symbol *tfs, Symbol *ifs)
{
    // Create an AgentFuncSymbol for the time function ('true' means the definition is developer-supplied)
    time_func = new AgentFuncSymbol(tfs, agent, "Time", "", true, decl_loc);
    time_func->doc_block = doxygen_short("Return the time to the event " + event_name + " in the " + agent->name + " agent (model code).");

    // Create an AgentFuncSymbol for the implement function ('true' means the definition is developer-supplied)
    implement_func = new AgentFuncSymbol(ifs, agent, "void", "", true, decl_loc);
    implement_func->doc_block = doxygen_short("Implement the event " + event_name + " when it occurs in the " + agent->name + " agent (model code).");
}

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

        // Add this agent event time symbol to the agent's list of all callback members
        pp_agent->pp_callback_members.push_back(this);
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
                    CodeBlock& c = av->side_effects_fn->func_body;
                    c += "// Recalculate time to event " + event_name;
                    c += "if (om_active) " + name + ".make_dirty();";
                }
            }
        }
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
    h += "Event<" + agent->name + ", "
        + to_string(event_id) + ", "
        + "0, " // TODO event priority
        + "&" + implement_func->unique_name + ", "
        + "&" + time_func->unique_name + ">";
    h += name + ";";

    return h;
}

