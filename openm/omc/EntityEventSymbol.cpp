/**
* @file    EntityEventSymbol.cpp
* Definitions for the EntityEventSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

//#include <cassert>
//#include <algorithm>
//#include <typeinfo>
#include <cassert>
#include <set>
#include "EntityEventSymbol.h"
#include "EntitySymbol.h"
#include "AttributeSymbol.h"
#include "BuiltinAttributeSymbol.h"
#include "EntityFuncSymbol.h"
#include "CodeBlock.h"

using namespace std;

void EntityEventSymbol::create_auxiliary_symbols(Symbol *tfs, Symbol *ifs, bool is_developer_supplied)
{
    if (is_developer_supplied) {
        // Create an EntityFuncSymbol for the time function ('true' means the definition is developer-supplied, so suppress definition)
        time_func = new EntityFuncSymbol(tfs, agent, "Time", "", true, decl_loc);
        time_func->doc_block = doxygen_short("Return the time to the event " + event_name + " in the " + agent->name + " agent (model code).");
        time_func_model_code = time_func;

        // Create an EntityFuncSymbol for the implement function ('true' means the definition is developer-supplied, so suppress definition)
        implement_func = new EntityFuncSymbol(ifs, agent, "void", "", true, decl_loc);
        implement_func->doc_block = doxygen_short("Implement the event " + event_name + " when it occurs in the " + agent->name + " agent (model code).");
    }
    else {
        // The functions are created internally (for the internally-generated self-scheduling event)
        time_func = dynamic_cast<EntityFuncSymbol *>(tfs);
        assert(time_func);
        implement_func = dynamic_cast<EntityFuncSymbol *>(ifs);
        assert(implement_func);
    }
}

void EntityEventSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eCreateMissingSymbols:
    {
        // First determine if this is an event with memory or not,
        // since if so the members of the event trace cover function symbols 
        // possibly created below will depend on that.
        bool time_func_defined = false;
        bool time_func_memory = false;
        {
            assert(time_func); // logic guarantee
            auto search = memfunc_parmlist.find(time_func->unique_name);
            if (search == memfunc_parmlist.end()) {
                time_func_defined = false; // this is an error
            }
            else {
                time_func_defined = true;
                auto parmlist = search->second;
                // see if time function parmlist starts with '( int *'
                if (parmlist.size() >= 2 && parmlist[0] == "int" && parmlist[1] == "*") {
                    time_func_memory = true;
                }
            }
        }

        bool implement_func_defined = false;
        bool implement_func_memory = false;
        {
            assert(implement_func); // logic guarantee
            auto search = memfunc_parmlist.find(implement_func->unique_name);
            if (search == memfunc_parmlist.end()) {
                implement_func_defined = false; // this is an error
            }
            else {
                implement_func_defined = true;
                auto parmlist = search->second;
                // see if implement function parmlist starts with '( int '
                if (parmlist.size() >= 2 && parmlist[0] == "int" && parmlist[1] != "*") {
                    implement_func_memory = true;
                }
            }
        }

        if (!time_func_defined) {
            pp_error(LT("error : missing definition for event time function '") + time_func->unique_name + LT("'"));
        }
        if (!implement_func_defined) {
            pp_error(LT("error : missing definition for event implement function '") + implement_func->unique_name + LT("'"));
        }
        if (time_func_memory != implement_func_memory) {
            pp_error(LT("error : mismatching arguments in event time function '") + time_func->unique_name + LT("' and event implement function '") + implement_func->unique_name + LT("'"));
        }

        // Now record the conclusion of the preceding chunk of code.
        event_memory = time_func_memory;

        if (event_memory) {
            // Change the declaration argument lists (were empty strings)
            time_func->arg_list_decl = "int *";
            implement_func->arg_list_decl = "int";
        }

        if (Symbol::option_event_trace) {
            // The event_trace option is on, so create cover functions
            // for the time and implement functions of the event,
            // and use them instead of the developer-supplied functions.
            // The cover functions will perform logging and then call
            // the original developer functions.

            // Create an EntityFuncSymbol for the time 'cover' function
            string cover_time_func_name = "om_cover_" + time_func->name;
            auto cover_time_func = new EntityFuncSymbol(
                cover_time_func_name,
                agent,
                "Time",
                event_memory ? "int * p_event_mem" : "",
                false
            );
            cover_time_func->doc_block = doxygen_short("Logging cover function: Return the time to the event " + event_name + " in the " + agent->name + " agent.");
            CodeBlock & ct = cover_time_func->func_body;
            ct += "Time event_time = " + time_func->name + (event_memory ? "(p_event_mem);" : "();");
            ct += "if (BaseEvent::trace_event_on) "
                "BaseEvent::event_trace_msg("
                "\"" + agent->name + "\", "
                "(int)entity_id, "
                "GetCaseSeed(), "
                "\"" + time_func->name + "\", "
                " (double)event_time);"
                ;
            ct += "return event_time;";
            // Plug it in
            time_func = cover_time_func;

            // Create an EntityFuncSymbol for the implement 'cover' function
            string cover_implement_func_name = "om_cover_" + implement_func->name;
            auto cover_implement_func = new EntityFuncSymbol(
                cover_implement_func_name,
                agent,
                "void",
                event_memory ? "int event_mem" : "",
                false
            );
            cover_implement_func->doc_block = doxygen_short("Logging cover function: Implement the event " + event_name + " when it occurs in the " + agent->name + " agent.");
            CodeBlock & ci = cover_implement_func->func_body;
            ci += "if (BaseEvent::trace_event_on) "
                "BaseEvent::event_trace_msg("
                "\"" + agent->name + "\", "
                "(int)entity_id, "
                "GetCaseSeed(), "
                "\"" + agent->name + "." + event_name + "\", "
                "time);"
                ;
            ci += implement_func->name + (event_memory ? "(event_mem);" : "();");
            // Plug it in
            implement_func = cover_implement_func;
            // push the names into the pass #1 ignore hash
            pp_ignore_pass1.insert(cover_time_func->unique_name);
            pp_ignore_pass1.insert(cover_implement_func->unique_name);

        }
        break;
    }
    case ePopulateCollections:
    {
        // Add this agent event time symbol to the agent's list of all such symbols
        pp_agent->pp_agent_events.push_back(this);

        // Add this agent event time symbol to the agent's list of all callback members
        pp_agent->pp_callback_members.push_back(this);
        break;
    }
    case ePopulateDependencies:
    {
        // Dependencies are generated directly if not developer-supplied.
        if (!is_developer_supplied) break;
        // Iterate through list of identifiers in the body of the time function
        // whose name matches an agentvar.
        for (auto identifier : time_func_model_code->body_identifiers) {
            if (exists(identifier, pp_agent)) {
                auto sym = get_symbol(identifier, pp_agent);
                auto av = dynamic_cast<AttributeSymbol *>(sym);
                if (av) {
                    // dependency of time function on av detected
                    CodeBlock& c = av->side_effects_fn->func_body;
                    c += injection_description();
                    c += "// Recalculate time to event " + event_name;
                    c += "if (om_active) " + name + ".make_dirty();";
                }
            }
        }
        break;
    }
    default:
    break;
    }
}

CodeBlock EntityEventSymbol::cxx_declaration_agent()
{
    // Hook into the hierarchical calling chain
    CodeBlock h = super::cxx_declaration_agent();

    // Perform operations specific to this level in the Symbol hierarchy.

    int event_id = pp_event_id;
    int modgen_event_num = pp_modgen_event_num;
    h += (event_memory ? "MemoryEvent<" : "Event<")
        + agent->name + ", "
        + to_string(event_id) + ", "
        + to_string(event_priority) + ", "
        + to_string(modgen_event_num) + ", "
        + "&" + implement_func->unique_name + ", "
        + "&" + time_func->unique_name + ">";
    h += name + ";";

    return h;
}

