/**
* @file    AgentSymbol.cpp
* Implements the AgentSymbol derived class of the Symbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include "AgentSymbol.h"
#include "BuiltinAgentVarSymbol.h"
#include "AgentInternalSymbol.h"
#include "AgentFuncSymbol.h"
#include "TableSymbol.h"
#include "NumericSymbol.h"

using namespace std;

void AgentSymbol::create_auxiliary_symbols()
{
    // Create builtin agentvars for this agent: time, age, events, agent_id
    if (!exists("time", this))
        new BuiltinAgentVarSymbol("time", this, NumericSymbol::find(token::TK_Time));
    if (!exists("age", this))
        new BuiltinAgentVarSymbol("age", this, NumericSymbol::find(token::TK_Time));
    if (!exists("events", this))
        new BuiltinAgentVarSymbol("events", this, NumericSymbol::find(token::TK_counter));
    if (!exists("agent_id", this))
        new BuiltinAgentVarSymbol("agent_id", this, NumericSymbol::find(token::TK_int));

    // TODO: Remove test - Create internal data members for this agent: allow_assignment
    if (!exists("allow_assignment", this))
        new AgentInternalSymbol("allow_assignment", this, Symbol::get_symbol(Symbol::token_to_string(token::TK_bool)));

    // The initialize_tables member function
    {
        assert(nullptr == initialize_tables_fn); // initialization guarantee
        // Ex. "om_prepare_increment_DurationOfLife"
        initialize_tables_fn = new AgentFuncSymbol("om_initialize_tables", this);
        assert(initialize_tables_fn); // out of memory check
        initialize_tables_fn->doc_block = doxygen_short("Initialize increment for each table in " + name + ".");
    }

    // The finalize_tables member function
    {
        assert(nullptr == finalize_tables_fn); // initialization guarantee
        // Ex. "om_prepare_increment_DurationOfLife"
        finalize_tables_fn = new AgentFuncSymbol("om_finalize_tables", this);
        assert(finalize_tables_fn); // out of memory check
        finalize_tables_fn->doc_block = doxygen_short("Finalize increment for each table in " + name + ".");
    }

    // The Start() member function
    {
        auto start_fn = dynamic_cast<AgentFuncSymbol *>(get_symbol("Start", this));
        if (!start_fn) start_fn = new AgentFuncSymbol("Start", this);
        assert(start_fn);
        start_fn->suppress_defn = false;
        start_fn->doc_block = doxygen_short("Start simulating the agent.");
        start_fn->func_body += "om_Start_begin();";
        start_fn->func_body += "om_Start_custom();";
        start_fn->func_body += "om_Start_end();";
    }

    // The Finish() member function
    {
        auto finish_fn = dynamic_cast<AgentFuncSymbol *>(get_symbol("Finish", this));
        if (!finish_fn) finish_fn = new AgentFuncSymbol("Finish", this);
        assert(finish_fn);
        finish_fn->suppress_defn = false;
        finish_fn->doc_block = doxygen_short("Finish simulating the agent.");
        finish_fn->func_body += "om_Finish_custom();";
        finish_fn->func_body += "om_Finish_end();";
    }
}

void AgentSymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case ePopulateCollections:
        {
            // Add this agent to the complete list of agents.
            pp_all_agents.push_back(this);

            // assign direct pointer to builtin member 'time' for use post-parse
            pp_time = dynamic_cast<BuiltinAgentVarSymbol *>(get_symbol("time", this));
            assert(pp_time); // parser guarantee
        }
        break;
    case ePopulateDependencies:
        {
            // construct the body of the initialize_tables function
            build_body_initialize_tables();
            // construct the body of the finalize_tables function
            build_body_finalize_tables();
        }
        break;
    default:
        break;
    }
}

void AgentSymbol::build_body_initialize_tables()
{
    CodeBlock& c = initialize_tables_fn->func_body;

    for (auto table : pp_agent_tables) {
        c += table->update_cell_fn->name + "();" ;
        c += table->prepare_increment_fn->name + "();" ;
    }
}

void AgentSymbol::build_body_finalize_tables()
{
    CodeBlock& c = finalize_tables_fn->func_body;

    for (auto table : pp_agent_tables) {
        c += table->process_increment_fn->name + "();";
    }
}

