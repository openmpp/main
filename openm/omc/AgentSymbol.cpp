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
#include "IdentityAgentVarSymbol.h"
#include "LinkAgentVarSymbol.h"
#include "AgentMultilinkSymbol.h"
#include "EntitySetSymbol.h"
#include "TableSymbol.h"
#include "NumericSymbol.h"
#include "BoolSymbol.h"

using namespace std;

void AgentSymbol::create_auxiliary_symbols()
{
    // Create builtin agentvars for this agent: time, age, events, entity_id
    if (!exists("time", this)) {
        auto time_sym = new BuiltinAgentVarSymbol("time", this, NumericSymbol::find(token::TK_Time));
        // declare the om_delta local variable for use in subsequently added code
        auto fn = time_sym->side_effects_fn;
        assert(fn);
        CodeBlock& c = fn->func_body;
        // The local variable om_delta can be used by any code injected to 'time'.
        c += "// Amount of time increment";
        c += "Time om_delta = om_new - om_old;";
    }
    if (!exists("age", this)) {
        auto age_sym = new BuiltinAgentVarSymbol("age", this, NumericSymbol::find(token::TK_Time));
        age_sym->sorting_group = 2; // continuously-updated
    }
    if (!exists("events", this)) {
        new BuiltinAgentVarSymbol("events", this, NumericSymbol::find(token::TK_counter));
    }
    if (!exists("entity_id", this)) {
        new BuiltinAgentVarSymbol("entity_id", this, NumericSymbol::find(token::TK_int));
    }

    // TODO: Remove test - Create internal data members for this agent: allow_assignment
    //if (!exists("allow_assignment", this)) {
    //    new AgentInternalSymbol("allow_assignment", this, BoolSymbol::find());
    //}

    // The age_agent() member function
    // Used in the run-time support classes
    {
        auto *fn = new AgentFuncSymbol("age_agent", this, "void", "Time t");
        fn->doc_block = doxygen_short("Age the entity to the given time.");
        CodeBlock& c = fn->func_body;
        c += "if (time <= t) time.set(t);";
        c += "else assert(false); // time running backwards?";
    }

    // The get_entity_id() member function
    {
        auto *fn = new AgentFuncSymbol("get_entity_id", this, "int", "");
        fn->doc_block = doxygen_short("Return unique entity_id of this entity.");
        CodeBlock& c = fn->func_body;
        c += "return entity_id;" ;
    }

    // The om_set_entity_id() member function
    {
        auto *fn = new AgentFuncSymbol("om_set_entity_id", this, "void", "");
        fn->doc_block = doxygen_short("Set the unique entity_id of this entity.");
        CodeBlock& c = fn->func_body;
        c += "entity_id.set(get_next_entity_id());" ;
    }

    // The om_set_start_time() member function
    {
        auto *fn = new AgentFuncSymbol("om_set_start_time", this, "void", "");
        fn->doc_block = doxygen_short("Set the default start time of this entity.");
        CodeBlock& c = fn->func_body;
        c += "time.set(BaseEvent::global_time);" ;
    }

    // The om_Start_custom() member function
    {
        // no code generation for definition
        auto *fn = new AgentFuncSymbol("om_Start_custom", this, "void", "", true);
        fn->doc_block = doxygen_short("Model-specific customizations before simulating entity.");
    }

    // The om_Finish_custom() member function
    {
        // no code generation for definition
        auto *fn = new AgentFuncSymbol("om_Finish_custom", this, "void", "", true);
        fn->doc_block = doxygen_short("Model-specific customizations after simulating entity.");
    }

    // The initialize_callback_member_offsets member function
    {
        assert(!initialize_callback_member_offsets_fn); // initialization guarantee
        initialize_callback_member_offsets_fn = new AgentFuncSymbol("om_initialize_callback_member_offsets", this);
        assert(initialize_callback_member_offsets_fn); // out of memory check
        initialize_callback_member_offsets_fn->doc_block = doxygen_short("One-time calculation of the offsets of agentvars in the containing entity.");
        // function body is generated in post-parse phase
    }

    // The initialize_data_members member function
    {
        assert(!initialize_data_members_fn); // initialization guarantee
        initialize_data_members_fn = new AgentFuncSymbol("om_initialize_data_members", this);
        assert(initialize_data_members_fn); // out of memory check
        initialize_data_members_fn->doc_block = doxygen_short("Initialization of data members before the entity enters simulation.");
        // function body is generated in post-parse phase
    }

    // The initialize_data_members0 member function
    {
        assert(!initialize_data_members0_fn); // initialization guarantee
        initialize_data_members0_fn = new AgentFuncSymbol("om_initialize_data_members0", this);
        assert(initialize_data_members0_fn); // out of memory check
        initialize_data_members0_fn->doc_block = doxygen_short("Initialization of data members of the 'zero' entity to default values for the type (0).");
        // function body is generated in post-parse phase
    }

    // The om_initialize_events member function
    {
        assert(nullptr == initialize_events_fn); // initialization guarantee
        initialize_events_fn = new AgentFuncSymbol("om_initialize_events", this);
        assert(initialize_events_fn); // out of memory check
        initialize_events_fn->doc_block = doxygen_short("Force event time calculation for each event in the entity when it enters simulation.");
        // function body is generated in post-parse phase
    }

    // The om_finalize_events member function
    {
        assert(nullptr == finalize_events_fn); // initialization guarantee
        finalize_events_fn = new AgentFuncSymbol("om_finalize_events", this);
        assert(finalize_events_fn); // out of memory check
        finalize_events_fn->doc_block = doxygen_short("Remove each event in the entity from the event queue when it leaves the simulation.");
        // function body is generated in post-parse phase
    }

    // The om_initialize_entity_sets member function
    {
        assert(nullptr == initialize_entity_sets_fn); // initialization guarantee
        initialize_entity_sets_fn = new AgentFuncSymbol("om_initialize_entity_sets", this);
        assert(initialize_entity_sets_fn); // out of memory check
        initialize_entity_sets_fn->doc_block = doxygen_short("Insert the entity in each entity set when it enters the simulation.");
        // function body is generated in post-parse phase
    }

    // The om_finalize_entity_sets member function
    {
        assert(nullptr == finalize_entity_sets_fn); // initialization guarantee
        finalize_entity_sets_fn = new AgentFuncSymbol("om_finalize_entity_sets", this);
        assert(finalize_entity_sets_fn); // out of memory check
        finalize_entity_sets_fn->doc_block = doxygen_short("Remove the entity in each entity set when it leaves the simulation.");
        // function body is generated in post-parse phase
    }

    // The om_initialize_tables member function
    {
        assert(nullptr == initialize_tables_fn); // initialization guarantee
        initialize_tables_fn = new AgentFuncSymbol("om_initialize_tables", this);
        assert(initialize_tables_fn); // out of memory check
        initialize_tables_fn->doc_block = doxygen_short("Initialize the entity's increments in each table when it enters the simulation.");
        // function body is generated in post-parse phase
    }

    // The om_finalize_tables member function
    {
        assert(nullptr == finalize_tables_fn); // initialization guarantee
        finalize_tables_fn = new AgentFuncSymbol("om_finalize_tables", this);
        assert(finalize_tables_fn); // out of memory check
        finalize_tables_fn->doc_block = doxygen_short("Finish the entity's pending increments in each table when it leaves the simulation.");
        // function body is generated in post-parse phase
    }

    // The initialize_expression_agentvars member function
    {
        assert(nullptr == initialize_expression_agentvars_fn); // initialization guarantee
        initialize_expression_agentvars_fn = new AgentFuncSymbol("om_initialize_expression_agentvars", this);
        assert(initialize_expression_agentvars_fn); // out of memory check
        initialize_expression_agentvars_fn->doc_block = doxygen_short("Initialize each expression agentvar before the entity enters the simulation.");
        // function body is generated in post-parse phase
    }

    // The finalize_links member function
    {
        assert(nullptr == finalize_links_fn); // initialization guarantee
        finalize_links_fn = new AgentFuncSymbol("om_finalize_links", this);
        assert(finalize_links_fn); // out of memory check
        finalize_links_fn->doc_block = doxygen_short("Set all links in agent to nullptr when the entity leaves the simulation.");
        // function body is generated in post-parse phase
    }

    // The finalize_multilinks member function
    {
        assert(nullptr == finalize_multilinks_fn); // initialization guarantee
        finalize_multilinks_fn = new AgentFuncSymbol("om_finalize_multilinks", this);
        assert(finalize_multilinks_fn); // out of memory check
        finalize_multilinks_fn->doc_block = doxygen_short("Empty all multilinks in agent when the entity leaves the simulation.");
        // function body is generated in post-parse phase
    }

    // The Start() member function
    {
        auto fn = dynamic_cast<AgentFuncSymbol *>(get_symbol("Start", this));
        if (!fn) fn = new AgentFuncSymbol("Start", this);
        assert(fn);
        fn->doc_block = doxygen_short("Start simulating the entity.");
        CodeBlock& c = fn->func_body;
        c += "om_Start_begin();";
        c += "om_Start_custom();";
        c += "om_Start_end();";
    }

    // The Finish() member function
    {
        auto fn = dynamic_cast<AgentFuncSymbol *>(get_symbol("Finish", this));
        if (!fn) fn = new AgentFuncSymbol("Finish", this);
        assert(fn);
        fn->doc_block = doxygen_short("Finish simulating the entity.");
        CodeBlock& c = fn->func_body;
        c += "om_Finish_custom();";
        c += "om_Finish_end();";
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
            build_body_initialize_callback_member_offsets();
            build_body_initialize_data_members();
            build_body_initialize_data_members0();
            build_body_initialize_events();
            build_body_finalize_events();
            build_body_initialize_entity_sets();
            build_body_finalize_entity_sets();
            build_body_initialize_tables();
            build_body_finalize_tables();
            build_body_initialize_expression_agentvars();
            build_body_finalize_links();
            build_body_finalize_multilinks();
        }
        break;
    default:
        break;
    }
}

void AgentSymbol::build_body_initialize_callback_member_offsets()
{
    CodeBlock& c = initialize_callback_member_offsets_fn->func_body;

    for ( auto dm : pp_callback_members ) {
        // e.g. age.offset_in_agent = (char *)&(this->age) - (char *)this;
        c += dm->name + ".offset_in_agent = (char *)&(this->" + dm->name + ") - (char *)this;" ;
    }
}


void AgentSymbol::build_body_initialize_data_members()
{
    CodeBlock& c = initialize_data_members_fn->func_body;

    for ( auto adm : pp_agent_data_members ) {
        c += adm->cxx_initialization_expression(false);
    }
}

void AgentSymbol::build_body_initialize_data_members0()
{
    CodeBlock& c = initialize_data_members0_fn->func_body;

    for ( auto adm : pp_agent_data_members ) {
        c += adm->cxx_initialization_expression(true);
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

void AgentSymbol::build_body_initialize_entity_sets()
{
    CodeBlock& c = initialize_entity_sets_fn->func_body;

    for (auto entity_set : pp_agent_entity_sets) {
        c += "// " + entity_set->name;
        // If the entity set filter is false at initialization, do nothing
        if (entity_set->filter) {
            c += "if (" + entity_set->filter->name + ") {" ;
        }
        c += entity_set->update_cell_fn->name + "();" ;
        c += entity_set->insert_fn->name + "();" ;
        if (entity_set->filter) {
            c += "}" ;
        }
        c += "";
    }
}

void AgentSymbol::build_body_finalize_entity_sets()
{
    CodeBlock& c = finalize_entity_sets_fn->func_body;

    for (auto entity_set : pp_agent_entity_sets) {
        c += "// " + entity_set->name;
        // If the entity set filter is false at finalization, do nothing
        if (entity_set->filter) {
            c += "if (" + entity_set->filter->name + ") {" ;
        }
        c += entity_set->erase_fn->name + "();";
        if (entity_set->filter) {
            c += "}" ;
        }
        c += "";
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
        c += table->prepare_increments_fn->name + "();" ;
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
        c += table->process_increments_fn->name + "();";
        if (table->filter) {
            c += "}" ;
        }
        c += "";
    }
}

void AgentSymbol::build_body_initialize_expression_agentvars()
{
    CodeBlock& c = initialize_expression_agentvars_fn->func_body;

    for ( auto eav : pp_identity_agentvars ) {
        c += eav->expression_fn->name + "();";
    }
}

void AgentSymbol::build_body_finalize_links()
{
    CodeBlock& c = finalize_links_fn->func_body;

    for ( auto lav : pp_link_agentvars ) {
        c += lav->name + ".set(nullptr);";
    }
}

void AgentSymbol::build_body_finalize_multilinks()
{
    CodeBlock& c = finalize_multilinks_fn->func_body;

    for ( auto mlm : pp_multilink_members ) {
        c += mlm->name + ".clear();";
    }
}

