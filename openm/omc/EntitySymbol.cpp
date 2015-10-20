/**
* @file    EntitySymbol.cpp
* Implements the EntitySymbol derived class of the Symbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <cassert>
#include "libopenm/omCommon.h"
#include "EntitySymbol.h"
#include "BuiltinAttributeSymbol.h"
#include "EntityInternalSymbol.h"
#include "EntityFuncSymbol.h"
#include "EntityEventSymbol.h"
#include "IdentityAttributeSymbol.h"
#include "LinkAttributeSymbol.h"
#include "EntityMultilinkSymbol.h"
#include "EntitySetSymbol.h"
#include "EntityTableSymbol.h"
#include "EntityIncrementSymbol.h"
#include "NumericSymbol.h"
#include "BoolSymbol.h"
#include "ModelTypeSymbol.h"

using namespace std;

void EntitySymbol::create_auxiliary_symbols()
{
    // Create builtin agentvars for this agent: time, age, events, entity_id.
    // Need to handle situation where the symbol exists but requires morphing.
    // This can occur if the symbol has been used in a table before the agent was declared.

    {
        string nm = "time";
        auto sym = Symbol::get_symbol(nm, this);
        if (!sym || sym->is_base_symbol()) {
            NumericSymbol *typ = NumericSymbol::find(token::TK_Time);
            BuiltinAttributeSymbol *biav = nullptr;
            if (!sym) {
                // create it
                biav = new BuiltinAttributeSymbol(nm, this, typ);
            }
            else {
                // morph it
                biav = new BuiltinAttributeSymbol(sym, this, typ);
            }
            assert(biav);
            // initialize it
            // declare the om_delta local variable for use in subsequently added code
            auto fn = biav->side_effects_fn;
            assert(fn);
            CodeBlock& c = fn->func_body;
            // The local variable om_delta can be used by any code injected into 'time'.
            c += "// Amount of time increment";
            c += "Time om_delta = om_new - om_old;";
        }
    }

    {
        string nm = "age";
        auto sym = Symbol::get_symbol(nm, this);
        if (!sym || sym->is_base_symbol()) {
            NumericSymbol *typ = NumericSymbol::find(token::TK_Time);
            BuiltinAttributeSymbol *biav = nullptr;
            if (!sym) {
                // create it
                biav = new BuiltinAttributeSymbol(nm, this, typ);
            }
            else {
                // morph it
                biav = new BuiltinAttributeSymbol(sym, this, typ);
            }
            assert(biav);
            // initialize it
            biav->sorting_group = 2; // age is continuously-updated
        }
    }

    {
        string nm = "events";
        auto sym = Symbol::get_symbol(nm, this);
        if (!sym || sym->is_base_symbol()) {
            NumericSymbol *typ = NumericSymbol::find(token::TK_counter);
            BuiltinAttributeSymbol *biav = nullptr;
            if (!sym) {
                // create it
                biav = new BuiltinAttributeSymbol(nm, this, typ);
            }
            else {
                // morph it
                biav = new BuiltinAttributeSymbol(sym, this, typ);
            }
            assert(biav);
            // initialize it
        }
    }

    {
        string nm = "entity_id";
        auto sym = Symbol::get_symbol(nm, this);
        if (!sym || sym->is_base_symbol()) {
            NumericSymbol *typ = NumericSymbol::find(token::TK_int);
            BuiltinAttributeSymbol *biav = nullptr;
            if (!sym) {
                // create it
                biav = new BuiltinAttributeSymbol(nm, this, typ);
            }
            else {
                // morph it
                biav = new BuiltinAttributeSymbol(sym, this, typ);
            }
            assert(biav);
            // initialize it
        }
    }

    {
        string nm = "case_seed";
        auto sym = Symbol::get_symbol(nm, this);
        if (!sym || sym->is_base_symbol()) {
            NumericSymbol *typ = NumericSymbol::find(token::TK_double);
            BuiltinAttributeSymbol *biav = nullptr;
            if (!sym) {
                // create it
                biav = new BuiltinAttributeSymbol(nm, this, typ);
            }
            else {
                // morph it
                biav = new BuiltinAttributeSymbol(sym, this, typ);
            }
            assert(biav);
            // initialize it
        }
    }

    // The age_agent() member function
    {
        auto *fn = new EntityFuncSymbol("age_agent", this, "void", "Time t");
        fn->doc_block = doxygen_short("Age the entity to the given time.");
        CodeBlock& c = fn->func_body;
        c += "if (time < t) {";
        c += "// Age the entity forward to the given time.";
        c += "time.set(t);";
        c += "}";
        c += "else {";
        c += "// The entity is already older than the given time.";
        c += "// This is normal if another agent is 'catching up' to this agent in its own events.";
        c += "// It is a model error if the current event is in this agent,";
        c += "// since that implies that an event is trying to make time run backwards for this agent.";
        c += "// This possibility is detected and handled outside of this function.";
        c += "}";
    }

    // The om_get_entity_id() member function
    {
        auto *fn = new EntityFuncSymbol("om_get_entity_id", this, "int", "");
        fn->doc_block = doxygen_short("Return unique entity_id of this entity.");
        CodeBlock& c = fn->func_body;
        c += "return entity_id;" ;
    }

    // The om_set_entity_id() member function
    {
        auto *fn = new EntityFuncSymbol("om_set_entity_id", this, "void", "");
        fn->doc_block = doxygen_short("Set the unique entity_id of this entity.");
        CodeBlock& c = fn->func_body;
        c += "entity_id.set(get_next_entity_id());" ;
    }

    // The om_initialize_time_and_age() member function
    {
        auto *fn = new EntityFuncSymbol("om_initialize_time_and_age", this, "void", "");
        fn->doc_block = doxygen_short("Initialize time and age for this entity.");
        CodeBlock& c = fn->func_body;
        c += "// Calling initialize() to set the values of time and age to time_infinite";
        c += "// ensures that their associated side-effects functions will be invoked";
        c += "// when set() is called immediately after.  This ensures that identity attributes";
        c += "// and derived attributes which depend on time and age have correct values";
        c += "// if they are used by model developer code in the Start() function before the";
        c += "// entity enters the simulation.";
        c += "time.initialize(time_infinite);";
        c += "time.set(BaseEvent::get_global_time());";
        c += "age.initialize(time_infinite);";
        c += "age.set(0);";
    }

    // The om_get_time() member function
    {
        auto *fn = new EntityFuncSymbol("om_get_time", this, "Time", "");
        fn->doc_block = doxygen_short("Return current time of this entity.");
        CodeBlock& c = fn->func_body;
        c += "return time.get();" ;
    }

    // The om_Start_custom() member function
    {
        // no code generation for definition
        auto *fn = new EntityFuncSymbol("om_Start_custom", this, "void", "", true);
        fn->doc_block = doxygen_short("Model-specific customizations before simulating entity.");
    }

    // The assign_member_offsets member function
    {
        assert(!assign_member_offsets_fn); // initialization guarantee
        assign_member_offsets_fn = new EntityFuncSymbol("om_assign_member_offsets", this);
        assert(assign_member_offsets_fn); // out of memory check
        assign_member_offsets_fn->doc_block = doxygen_short("One-time calculation of the offsets of attributes and increments in the containing entity.");
        // function body is generated in post-parse phase
    }

    // The initialize_data_members member function
    {
        assert(!initialize_data_members_fn); // initialization guarantee
        initialize_data_members_fn = new EntityFuncSymbol("om_initialize_data_members", this);
        assert(initialize_data_members_fn); // out of memory check
        initialize_data_members_fn->doc_block = doxygen_short("Initialization of data members before the entity enters simulation.");
        // function body is generated in post-parse phase
    }

    // The initialize_data_members0 member function
    {
        assert(!initialize_data_members0_fn); // initialization guarantee
        initialize_data_members0_fn = new EntityFuncSymbol("om_initialize_data_members0", this);
        assert(initialize_data_members0_fn); // out of memory check
        initialize_data_members0_fn->doc_block = doxygen_short("Initialization of data members of the 'zero' entity to default values for the type (0).");
        // function body is generated in post-parse phase
    }

    // The om_initialize_events member function
    {
        assert(nullptr == initialize_events_fn); // initialization guarantee
        initialize_events_fn = new EntityFuncSymbol("om_initialize_events", this);
        assert(initialize_events_fn); // out of memory check
        initialize_events_fn->doc_block = doxygen_short("Force event time calculation for each event in the entity when it enters simulation.");
        // function body is generated in post-parse phase
    }

    // The om_finalize_events member function
    {
        assert(nullptr == finalize_events_fn); // initialization guarantee
        finalize_events_fn = new EntityFuncSymbol("om_finalize_events", this);
        assert(finalize_events_fn); // out of memory check
        finalize_events_fn->doc_block = doxygen_short("Remove each event in the entity from the event queue when it leaves the simulation.");
        // function body is generated in post-parse phase
    }

    // The om_initialize_entity_sets member function
    {
        assert(nullptr == initialize_entity_sets_fn); // initialization guarantee
        initialize_entity_sets_fn = new EntityFuncSymbol("om_initialize_entity_sets", this);
        assert(initialize_entity_sets_fn); // out of memory check
        initialize_entity_sets_fn->doc_block = doxygen_short("Insert the entity in each entity set when it enters the simulation.");
        // function body is generated in post-parse phase
    }

    // The om_finalize_entity_sets member function
    {
        assert(nullptr == finalize_entity_sets_fn); // initialization guarantee
        finalize_entity_sets_fn = new EntityFuncSymbol("om_finalize_entity_sets", this);
        assert(finalize_entity_sets_fn); // out of memory check
        finalize_entity_sets_fn->doc_block = doxygen_short("Remove the entity in each entity set when it leaves the simulation.");
        // function body is generated in post-parse phase
    }

    // The om_initialize_tables member function
    {
        assert(nullptr == initialize_tables_fn); // initialization guarantee
        initialize_tables_fn = new EntityFuncSymbol("om_initialize_tables", this);
        assert(initialize_tables_fn); // out of memory check
        initialize_tables_fn->doc_block = doxygen_short("Initialize the entity's increments in each table when it enters the simulation.");
        // function body is generated in post-parse phase
    }

    // The om_finalize_tables member function
    {
        assert(nullptr == finalize_tables_fn); // initialization guarantee
        finalize_tables_fn = new EntityFuncSymbol("om_finalize_tables", this);
        assert(finalize_tables_fn); // out of memory check
        finalize_tables_fn->doc_block = doxygen_short("Finish the entity's pending increments in each table when it leaves the simulation.");
        // function body is generated in post-parse phase
    }

    // The initialize_identity_attributes_fn member function
    {
        assert(nullptr == initialize_identity_attributes_fn); // initialization guarantee
        initialize_identity_attributes_fn = new EntityFuncSymbol("om_initialize_identity_attributes", this);
        assert(initialize_identity_attributes_fn); // out of memory check
        initialize_identity_attributes_fn->doc_block = doxygen_short("Initialize each identity attribute before developer code in Start executes.");
        // function body is generated through code injection by attributes in post-parse phase
    }

    // The initialize_derived_attributes_fn member function
    {
        assert(nullptr == initialize_derived_attributes_fn); // initialization guarantee
        initialize_derived_attributes_fn = new EntityFuncSymbol("om_initialize_derived_attributes", this);
        assert(initialize_derived_attributes_fn); // out of memory check
        initialize_derived_attributes_fn->doc_block = doxygen_short("Initialize derived attributes before the entity enters the simulation.");
        // function body is generated through code injection by attributes in post-parse phase
    }

    // The reset_derived_attributes_fn member function
    {
        assert(nullptr == reset_derived_attributes_fn); // initialization guarantee
        reset_derived_attributes_fn = new EntityFuncSymbol("om_reset_derived_attributes", this);
        assert(reset_derived_attributes_fn); // out of memory check
        reset_derived_attributes_fn->doc_block = doxygen_short("Reset derived attributes before the entity enters the simulation.");
        // function body is generated through code injection by attributes in post-parse phase
    }

    // The finalize_links member function
    {
        assert(nullptr == finalize_links_fn); // initialization guarantee
        finalize_links_fn = new EntityFuncSymbol("om_finalize_links", this);
        assert(finalize_links_fn); // out of memory check
        finalize_links_fn->doc_block = doxygen_short("Set all links in agent to nullptr when the entity leaves the simulation.");
        // function body is generated in post-parse phase
    }

    // The finalize_multilinks member function
    {
        assert(nullptr == finalize_multilinks_fn); // initialization guarantee
        finalize_multilinks_fn = new EntityFuncSymbol("om_finalize_multilinks", this);
        assert(finalize_multilinks_fn); // out of memory check
        finalize_multilinks_fn->doc_block = doxygen_short("Empty all multilinks in agent when the entity leaves the simulation.");
        // function body is generated in post-parse phase
    }
}

void EntitySymbol::post_parse(int pass)
{
    // Hook into the post_parse hierarchical calling chain
    super::post_parse(pass);

    // Perform post-parse operations specific to this level in the Symbol hierarchy.
    switch (pass) {
    case eCreateMissingSymbols:
    {
        // Find the one and only ModelTypeSymbol
        auto mts = ModelTypeSymbol::find();
        assert(mts);
        if (mts->is_case_based()) {
            // create case_id (or morph it to the correct type)
            //theLog->logFormatted("creating case_id for entity %s", name.c_str());
            string nm = "case_id";
            auto sym = Symbol::get_symbol(nm, this);
            if (!sym || sym->is_base_symbol()) {
                NumericSymbol *typ = NumericSymbol::find(token::TK_llong);
                BuiltinAttributeSymbol *biav = nullptr;
                if (!sym) {
                    // create it
                    biav = new BuiltinAttributeSymbol(nm, this, typ);
                }
                else {
                    // morph it
                    biav = new BuiltinAttributeSymbol(sym, this, typ);
                }
                // push its name into the pass #1 ignore hash
                pp_ignore_pass1.insert(biav->unique_name);
            }
        }
        break;
    }

    case eAssignMembers:
    {
        // assign direct pointer to builtin member 'time' for use post-parse
        pp_time = dynamic_cast<BuiltinAttributeSymbol *>(get_symbol("time", this));
        assert(pp_time); // parser guarantee
        break;
    }
    case ePopulateCollections:
    {
        // Add this agent to the complete list of agents.
        pp_all_agents.push_back(this);
        break;
    }
    case ePopulateDependencies:
    {
        // construct function bodies.
        build_body_assign_member_offsets();
        build_body_initialize_data_members();
        build_body_initialize_data_members0();
        build_body_initialize_events();
        build_body_finalize_events();
        build_body_initialize_entity_sets();
        build_body_finalize_entity_sets();
        build_body_initialize_tables();
        build_body_finalize_tables();
        build_body_finalize_links();
        build_body_finalize_multilinks();
        break;
    }
    default:
        break;
    }
}

void EntitySymbol::build_body_assign_member_offsets()
{
    CodeBlock& c = assign_member_offsets_fn->func_body;

    for ( auto dm : pp_callback_members ) {
        // e.g. age.offset_in_agent = (char *)&(this->age) - (char *)this;
        c += dm->name + ".offset_in_agent = (char *)&(this->" + dm->name + ") - (char *)this;" ;
    }
}


void EntitySymbol::build_body_initialize_data_members()
{
    CodeBlock& c = initialize_data_members_fn->func_body;

    for ( auto adm : pp_agent_data_members ) {
        c += adm->cxx_initialization_expression(false);
    }

    auto mts = ModelTypeSymbol::find();
    assert(mts);
    if (mts->is_case_based()) {
        c += "";
        c += "case_id.initialize(GetCaseID());";
        c += "case_seed.initialize(GetCaseSeed());";
    }
}

void EntitySymbol::build_body_initialize_data_members0()
{
    CodeBlock& c = initialize_data_members0_fn->func_body;

    for ( auto adm : pp_agent_data_members ) {
        c += adm->cxx_initialization_expression(true);
    }
}

void EntitySymbol::build_body_initialize_events()
{
    CodeBlock& c = initialize_events_fn->func_body;

    for ( auto event : pp_agent_events ) {
        c += event->name + ".make_dirty();";
    }
}

void EntitySymbol::build_body_finalize_events()
{
    CodeBlock& c = finalize_events_fn->func_body;

    for ( auto event : pp_agent_events ) {
        c += event->name + ".make_zombie();";
    }
}

void EntitySymbol::build_body_initialize_entity_sets()
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

void EntitySymbol::build_body_finalize_entity_sets()
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

void EntitySymbol::build_body_initialize_tables()
{
    CodeBlock& c = initialize_tables_fn->func_body;

    for (auto tbl : pp_entity_tables) {
        c += "// " + tbl->name;
        c += "{";
        if (tbl->filter) {
            c += "auto & filter = " + tbl->filter->name + ";";
        }
        else {
            c += "const bool filter = true; // table has no filter";
        }
        c += "auto cell = " + tbl->current_cell_fn->name + "();";
        c += "";
        c += "auto & incr = " + tbl->increment->name + ";";
        c += "incr.set_filter(filter);";
        c += "incr.set_cell(cell);";
        c += "incr.initialize_increment();";
        c += "}";
        c += "";
    }
}

void EntitySymbol::build_body_finalize_tables()
{
    CodeBlock& c = finalize_tables_fn->func_body;

    for (auto tbl : pp_entity_tables) {
        c += "// " + tbl->name;
        c += "{";
        c += "auto & incr = " + tbl->increment->name + ";";
        c += "incr.finalize_increment();";
        c += "}";
        c += "";
    }
}

void EntitySymbol::build_body_finalize_links()
{
    CodeBlock& c = finalize_links_fn->func_body;

    for ( auto lav : pp_link_agentvars ) {
        c += lav->name + ".set(nullptr);";
    }
}

void EntitySymbol::build_body_finalize_multilinks()
{
    CodeBlock& c = finalize_multilinks_fn->func_body;

    for ( auto mlm : pp_multilink_members ) {
        c += mlm->name + ".clear();";
    }
}

void EntitySymbol::create_ss_event()
{
    // check if the self-scheduling event has already been created
    if (ss_time_fn) return;

    // create the associated event time function
    ss_time_fn = new EntityFuncSymbol("om_ss_time", this, "Time", "");
    ss_time_fn->doc_block = doxygen_short("Time function for the self-scheduling event.");
    CodeBlock& ct = ss_time_fn->func_body;
    ct += "// Compute the minimum next time of all of the self-scheduling derived attributes." ;
    ct += "Time et = time_infinite;" ;
    // Subsequent code injected by each self-scheduling derived attribute
    // will min the corresponding self-scheduling time against the local variable 'et'.
    // The agent will inject a final line of code "return et;".  This final line is generated by finish_ss_event().

    // create the associated implement time function
    ss_implement_fn = new EntityFuncSymbol("om_ss_event", this, "void", "");
    ss_implement_fn->doc_block = doxygen_short("Implement function for the self-scheduling event.");
    CodeBlock& ci = ss_implement_fn->func_body;
    ci += "// Working local variable" ;
    ci += "Time current_time = time.get();";
    ci += "" ;
    ci += "// Update the value and next time of each self-scheduling derived attribute" ;
    ci += "// whose time has come." ;
    // Subsequent code will be injected by each self-scheduling derived attribute.
    
    // Create the event
    // The leading 'zzz' is a hack to make the self-scheduling event
    // sort after other events, to generate (under some conditions) comparable case checksums as Modgen.
    string evnt_name = "zzz_om_" + ss_implement_fn->name + "_om_event";
    ss_event = new EntityEventSymbol(evnt_name, this, ss_time_fn, ss_implement_fn, false, openm::event_priority_self_scheduling, decl_loc);
}

void EntitySymbol::finish_ss_event()
{
    if (ss_time_fn) {
        CodeBlock& ct = ss_time_fn->func_body;
        ct += injection_description();;
        ct += "return et;" ;
    }
}