/**
* @file    EntitySymbol.h
* Declares the EntitySymbol derived class of the Symbol class.
*/
// Copyright (c) 2013-2022 OpenM++ contributors
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <list>
#include <map>
#include "Symbol.h"

class EntityDataMemberSymbol;
class EntityEventSymbol;
class EntityEventSymbol;
class EntityFuncSymbol;
class AttributeSymbol;
class MaintainedAttributeSymbol;
class IdentityAttributeSymbol;
class LinkAttributeSymbol;
class EntityMultilinkSymbol;
class EntityInternalSymbol;
class BuiltinAttributeSymbol;

using namespace std;

/**
* EntitySymbol.
*/
class EntitySymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    EntitySymbol(Symbol *sym, omc::location decl_loc = omc::location())
        : Symbol(sym, decl_loc)
        , pp_time(nullptr)
        , assign_member_offsets_fn(nullptr)
        , initialize_data_members_fn(nullptr)
        , initialize_data_members0_fn(nullptr)
        , initialize_events_fn(nullptr)
        , finalize_events_fn(nullptr)
        , initialize_entity_sets_fn(nullptr)
        , finalize_entity_sets_fn(nullptr)
        , initialize_tables_fn(nullptr)
        , finalize_tables_fn(nullptr)
        , initialize_identity_attributes_fn(nullptr)
        , initialize_derived_attributes_fn(nullptr)
        , reset_derived_attributes_fn(nullptr)
        , finalize_links_fn(nullptr)
        , finalize_multilinks_fn(nullptr)
        , start_trace_fn(nullptr)
        , ss_time_fn(nullptr)
        , ss_implement_fn(nullptr)
        , get_entity_key_fn(nullptr)
        , get_microdata_key_fn(nullptr)
        , ss_event(nullptr)
        , next_ss_id(0)
        , pp_entity_id(-1)
        , pp_entity_has_rng_streams(false)
    {
        create_auxiliary_symbols();
        // Place in earliest sorting group so that missing symbols. e.g. case_id
        // are created or morphed before other symbols in first post_parse pass.
        sorting_group = 0;
    }

    /**
     * Create auxiliary symbols associated with this agent.
     */
    void create_auxiliary_symbols();

    void post_parse(int pass);

    /** Mark enumerations required for metadata support for this entity. */
    void post_parse_mark_enumerations(void);

    void populate_metadata(openm::MetaModelHolder & metaRows);

    /**
     * Builds the function body of the function.
     */
    void build_body_assign_member_offsets();

    /**
     * Builds the function body of the function.
     */
    void build_body_initialize_data_members();

    /**
     * Builds the function body of the function.
     */
    void build_body_initialize_data_members0();

    /**
     * Builds the function body of the function.
     */
    void build_body_initialize_events();

    /**
     * Builds the function body of the function.
     */
    void build_body_finalize_events();

    /**
     * Builds the function body of the function.
     */
    void build_body_initialize_entity_sets();

    /**
     * Builds the function body of the function.
     */
    void build_body_finalize_entity_sets();

    /**
     * Builds the function body of the function.
     */
    void build_body_initialize_tables();

    /**
     * Builds the function body of the function.
     */
    void build_body_finalize_tables();

    /**
     * Builds the function body of the function.
     */
    void build_body_finalize_links();

    /**
     * Builds the function body of the function.
     */
    void build_body_finalize_multilinks();

    /**
     * Builds the function body of the function.
     */
    void build_body_start_trace();

    /**
     * The built-in attribute for time in the agent.
     */
    BuiltinAttributeSymbol *pp_time;

    /**
     * Assigns offsets of attributes and increments within the entity.
     * 
     * This function has the fixed name om_assign_member_offsets().  It is called in the run-
     * time support class BaseEntity exactly once for each kind of entity.
     */
    EntityFuncSymbol *assign_member_offsets_fn;

    /**
     * The agent function which initializes the values of all data members in the agent.
     * 
     * This function has the fixed name om_initialize_data_members().  It is called in the run-time
     * support class BaseEntity before the agent enters the simulation.
     */
    EntityFuncSymbol *initialize_data_members_fn;

    /**
     * The agent function which initializes the values of all data members in the agent to type
     * default values.
     * 
     * This function has the fixed name om_initialize_data_members().  It is called in the run-time
     * support class BaseEntity before the agent enters the simulation.
     */
    EntityFuncSymbol *initialize_data_members0_fn;

    /**
     * The agent function which initializes all events in the agent.
     * 
     * This function has the fixed name om_initialize_events().  It is used in the run-time support
     * class BaseEntity before the agent enters the simulation.
     */
    EntityFuncSymbol *initialize_events_fn;

    /**
     * The agent function which finalizes all events in the agent.
     * 
     * This function has the fixed name om_events_tables(). It is used in the run-time support class
     * BaseEntity before the agent leaves the simulation.
     */
    EntityFuncSymbol *finalize_events_fn;

    /**
     * The agent function which initializes all entity sets in the agent.
     * 
     * This function has the fixed name om_initialize_entity_sets().  It is used in the run-time support
     * class BaseEntity before the agent enters the simulation.
     */
    EntityFuncSymbol *initialize_entity_sets_fn;

    /**
     * The agent function which finalizes all entity sets in the agent.
     * 
     * This function has the fixed name om_finalize_entity_sets(). It is used in the run-time support
     * class BaseEntity before the agent leaves the simulation.
     */
    EntityFuncSymbol *finalize_entity_sets_fn;

    /**
     * The agent function which initializes all tables in the agent.
     * 
     * This function has the fixed name om_initialize_tables().  It is used in the run-time support
     * class BaseEntity before the agent enters the simulation.
     */
    EntityFuncSymbol *initialize_tables_fn;

    /**
     * The agent function which finalizes all tables in the agent.
     * 
     * This function has the fixed name om_finalize_tables(). It is used in the run-time support
     * class BaseEntity before the agent leaves the simulation.
     */
    EntityFuncSymbol *finalize_tables_fn;

    /**
     * The agent function which initializes all identity attributes in the agent.
     * 
     * This function has the fixed name om_initialize_identity_attributes().  It is used in the run-
     * time support class BaseEntity before the agent enters the simulation.
     */
    EntityFuncSymbol *initialize_identity_attributes_fn;

    /**
     * The agent function which initializes derived attributes in the agent.
     * 
     * This function has the fixed name om_initialize_derived_attributes().  It is used in the run-
     * time support class BaseEntity to initialize values as part of the entity lifecycle.
     */
    EntityFuncSymbol *initialize_derived_attributes_fn;

    /**
     * The agent function which resets derived attributes in the agent.
     * 
     * This function has the fixed name om_reset_derived_attributes().  It is used in the run-
     * time support class BaseEntity as part of the entity lifecycle.
     */
    EntityFuncSymbol *reset_derived_attributes_fn;

    /**
     * The agent function which sets all links to nullptr when the agent finishes.
     */
    EntityFuncSymbol *finalize_links_fn;

    /**
     * The agent function which empties all multilinks when the agent finishes.
     */
    EntityFuncSymbol *finalize_multilinks_fn;

    /**
     * The entity function which implements trace messages at the start of the entity lifecycle.
     */
    EntityFuncSymbol* start_trace_fn;

    /**
     * The data members of this agent
     * 
     *  Populated after parsing is complete.
     */
    list<EntityDataMemberSymbol *> pp_agent_data_members;

    /**
     * The data members of this agent which use callbacks to implement side-effects.
     * 
     *  Populated after parsing is complete.
     */
    list<EntityMemberSymbol *> pp_callback_members;

    /**
     * The maintained attributes of this agent.
     * 
     *  Populated after parsing is complete.
     */
    list<MaintainedAttributeSymbol *> pp_maintained_attributes;

    /**
     * The expression attributes of this agent.
     * 
     * Populated after parsing is complete.
     */
    list<IdentityAttributeSymbol *> pp_identity_attributes;

    /**
     * The agent events of this agent
     * 
     *  Populated after parsing is complete.
     */
    list<EntityEventSymbol *> pp_agent_events;

    /**
     * The agent funcs of this agent
     * 
     *  Populated after parsing is complete.
     */
    list<EntityFuncSymbol *> pp_agent_funcs;

    /**
     * The enity sets of this agent
     * 
     *  Populated after parsing is complete.
     */
    list<EntitySetSymbol *> pp_agent_entity_sets;

    /**
     * The tables of this entity
     * 
     *  Populated after parsing is complete.
     */
    list<EntityTableSymbol *> pp_entity_tables;

    /**
     * The link attributes of this agent
     * 
     *  Populated after parsing is complete.
     */
    list<LinkAttributeSymbol *> pp_link_attributes;

    /**
     * The multilink members of this agent
     * 
     *  Populated after parsing is complete.
     */
    list<EntityMultilinkSymbol *> pp_multilink_members;

    /**
     * The hooks in the agent.
     * 
     * The key of the multimap is the name of the 'to' function of the hook.  The value of the multimap is the
     * name of the 'from' function.
     */
    multimap<string,string> pp_hooks;

    /**
     * The hooks in the agent, indexed by name and order
     * 
     * The key of the multimap is the name of the 'to' function of the hook, followed by the order.
     * The value of the multimap is the name of the 'from' function.
     */
    multimap<string,string> pp_hooks_with_order;

    /**
     * The RNG streams used in the entity
     *
     * Includes RNG streams used in the entity member functions,
     * e.g. calls like RandUniform(12)
     * as well as use of RNG streams in entity sets,
     * e.g. 
     */
    set<int> pp_rng_streams;

    /**
     * Create event to support self-scheduling derived attributes.
     * 
     * If not already created, create an event symbol and associated event time and event implement
     * function symbols to support self-scheduling derived attributes.  Self-scheduling derived
     * attributes will inject code into these functions to implement themselves.
     */
    void create_ss_event();

    /**
     * If the self-scheduling event exists, injects final code
     */
    void finish_ss_event();

    /**
     * The time function of the self-scheduling event.
     */
    EntityFuncSymbol *ss_time_fn;

    /**
     * The implement function of the self-scheduling event.
     */
    EntityFuncSymbol *ss_implement_fn;

    /**
     * The get_entity_ley function.
     */
    EntityFuncSymbol* get_entity_key_fn;

    /**
     * The get_microdata_key() function.
     */
    EntityFuncSymbol* get_microdata_key_fn;

    /**
     * The self-scheduling event.
     */
    EntityEventSymbol *ss_event;

    /**
     * Used to assign numeric identifiers to self-scheduling attributes in post-parse processing
     */
    int next_ss_id;

    /**
     * Numeric identifier. Used for communicating with metadata API.
     */
    int pp_entity_id;

    /**
     * Indicates if RNG streams are maintained in each entity of this kind.
     */
    bool pp_entity_has_rng_streams;
};

