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
#include "AgentEventSymbol.h"
#include "ExpressionAgentVarSymbol.h"
#include "TableSymbol.h"
#include "NumericSymbol.h"
#include "BoolSymbol.h"

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
        new AgentInternalSymbol("allow_assignment", this, BoolSymbol::find());

    // The age_agent() member function
    // Used in the run-time support classes
    {
        auto *fn = new AgentFuncSymbol("age_agent", this, "void", "Time t");
        fn->doc_block = doxygen_short("Age the agent to the given time.");
        fn->func_body += "time.set(t);";
    }

    // The get_agent_id() member function
    {
        auto *fn = new AgentFuncSymbol("get_agent_id", this, "int", "");
        fn->doc_block = doxygen_short("Return unique agent_id of this agent.");
        fn->func_body += "return agent_id.get();" ;
    }

    // The om_Start_custom() member function
    {
        auto *fn = new AgentFuncSymbol("om_Start_custom", this, "void", "", true);
        fn->doc_block = doxygen_short("Model-specific customizations before simulating agent.");
    }

    // The om_Finish_custom() member function
    {
        auto *fn = new AgentFuncSymbol("om_Finish_custom", this, "void", "", true);
        fn->doc_block = doxygen_short("Model-specific customizations after simulating agent.");
    }

    // The initialize_agentvar_offsets member function
    {
        assert(!initialize_agentvar_offsets_fn); // initialization guarantee
        initialize_agentvar_offsets_fn = new AgentFuncSymbol("om_initialize_agentvar_offsets", this);
        assert(initialize_agentvar_offsets_fn); // out of memory check
        initialize_agentvar_offsets_fn->doc_block = doxygen_short("One-time calculation of the offsets of agentvars in the containing agent.");
    }

    // The initialize_event_offsets member function
    {
        assert(!initialize_event_offsets_fn); // initialization guarantee
        initialize_event_offsets_fn = new AgentFuncSymbol("om_initialize_event_offsets", this);
        assert(initialize_event_offsets_fn); // out of memory check
        initialize_event_offsets_fn->doc_block = doxygen_short("One-time calculation of the offsets of events in the containing agent.");
    }

    // The initialize_data_members member function
    {
        assert(!initialize_data_members_fn); // initialization guarantee
        initialize_data_members_fn = new AgentFuncSymbol("om_initialize_data_members", this);
        assert(initialize_data_members_fn); // out of memory check
        initialize_data_members_fn->doc_block = doxygen_short("Initialization of data members before the agent enters simulation.");
    }

    // The om_initialize_events member function
    {
        assert(nullptr == initialize_events_fn); // initialization guarantee
        initialize_events_fn = new AgentFuncSymbol("om_initialize_events", this);
        assert(initialize_events_fn); // out of memory check
        initialize_events_fn->doc_block = doxygen_short("Force event time calculation for each event in the agent when it enters simulation.");
    }

    // The om_finalize_events member function
    {
        assert(nullptr == finalize_events_fn); // initialization guarantee
        finalize_events_fn = new AgentFuncSymbol("om_finalize_events", this);
        assert(finalize_events_fn); // out of memory check
        finalize_events_fn->doc_block = doxygen_short("Remove each event in the agent from the event queue when it leaves the simulation.");
    }

    // The om_initialize_tables member function
    {
        assert(nullptr == initialize_tables_fn); // initialization guarantee
        initialize_tables_fn = new AgentFuncSymbol("om_initialize_tables", this);
        assert(initialize_tables_fn); // out of memory check
        initialize_tables_fn->doc_block = doxygen_short("Initialize the agent's increment in each table when it enters the simulation.");
    }

    // The om_finalize_tables member function
    {
        assert(nullptr == finalize_tables_fn); // initialization guarantee
        finalize_tables_fn = new AgentFuncSymbol("om_finalize_tables", this);
        assert(finalize_tables_fn); // out of memory check
        finalize_tables_fn->doc_block = doxygen_short("Finish the agent's pending increment in each table when it leaves the simulation.");
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
            // construct function bodies.
            build_body_initialize_agentvar_offsets();
            build_body_initialize_event_offsets();
            build_body_initialize_data_members();
            build_body_initialize_events();
            build_body_finalize_events();
            build_body_initialize_tables();
            build_body_finalize_tables();
        }
        break;
    default:
        break;
    }
}

void AgentSymbol::build_body_initialize_agentvar_offsets()
{
    CodeBlock& c = initialize_agentvar_offsets_fn->func_body;

    for ( auto av : pp_agentvars ) {
        // e.g. age.offset_in_agent = (char *)&(this->age) - (char *)this;
        c += av->name + ".offset_in_agent = (char *)&(this->" + av->name + ") - (char *)this;" ;
    }
}

void AgentSymbol::build_body_initialize_event_offsets()
{
    CodeBlock& c = initialize_event_offsets_fn->func_body;

    for ( auto event : pp_agent_events ) {
        // e.g. om_time_MortalityEvent.offset_in_agent = (char *)&(this->om_time_MortalityEvent) - (char *)this;
        c += event->name + ".offset_in_agent = (char *)&(this->" + event->name + ") - (char *)this;";
    }
}

void AgentSymbol::build_body_initialize_data_members()
{
    CodeBlock& c = initialize_data_members_fn->func_body;

    c += "// Assign default initial value to all data members";
    for ( auto adm : pp_agent_data_members ) {
        c += adm->cxx_initialize_expression();
    }

    c += "";
    c += "// Evaluate expression agentvars";
    for ( auto eav : pp_expr_agentvars ) {
        c += eav->expression_fn->name + "();";
    }
}

void AgentSymbol::build_body_initialize_events()
{
    CodeBlock& c = initialize_events_fn->func_body;

    for ( auto event : pp_agent_events ) {
        c += event->name + ".make_dirty();";
    }
}

void AgentSymbol::build_body_finalize_events()
{
    CodeBlock& c = finalize_events_fn->func_body;

    for ( auto event : pp_agent_events ) {
        c += event->name + ".make_zombie();";
    }
}

void AgentSymbol::build_body_initialize_tables()
{
    CodeBlock& c = initialize_tables_fn->func_body;

    for (auto table : pp_agent_tables) {
        c += "// " + table->name;
        // If the table filter is false at initialization, do nothing
        if (table->filter) {
            c += "if (" + table->filter->name + ") {" ;
        }
        c += table->update_cell_fn->name + "();" ;
        c += table->prepare_increment_fn->name + "();" ;
        if (table->filter) {
            c += "}" ;
        }
        c += "";
    }
}

void AgentSymbol::build_body_finalize_tables()
{
    CodeBlock& c = finalize_tables_fn->func_body;

    for (auto table : pp_agent_tables) {
        c += "// " + table->name;
        // If the table filter is false at finalization, do nothing
        if (table->filter) {
            c += "if (" + table->filter->name + ") {" ;
        }
        c += table->process_increment_fn->name + "();";
        if (table->filter) {
            c += "}" ;
        }
        c += "";
    }
}

