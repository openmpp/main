/**
* @file    EntitySymbol.h
* Declares the EntitySymbol derived class of the Symbol class.
*/
// Copyright (c) 2013-2015 OpenM++
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
    EntitySymbol(Symbol *sym, yy::location decl_loc = yy::location())
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
        , ss_time_fn(nullptr)
        , ss_implement_fn(nullptr)
        , ss_event(nullptr)
        , next_ss_id(0)
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
     * The built-in agentvar for time in the agent.
     */
    BuiltinAttributeSymbol *pp_time;

    /**
     * Assigns offsets of attributes and increments within the entity.
     * 
     * This function has the fixed name om_assign_member_offsets().  It is called in the run-
     * time support class BaseAgent exactly once for each kind of entity.
     */
    EntityFuncSymbol *assign_member_offsets_fn;

    /**
     * The agent function which initializes the values of all data members in the agent.
     * 
     * This function has the fixed name om_initialize_data_members().  It is called in the run-time
     * support class BaseAgent before the agent enters the simulation.
     */
    EntityFuncSymbol *initialize_data_members_fn;

    /**
     * The agent function which initializes the values of all data members in the agent to type
     * default values.
     * 
     * This function has the fixed name om_initialize_data_members().  It is called in the run-time
     * support class BaseAgent before the agent enters the simulation.
     */
    EntityFuncSymbol *initialize_data_members0_fn;

    /**
     * The agent function which initializes all events in the agent.
     * 
     * This function has the fixed name om_initialize_events().  It is used in the run-time support
     * class BaseAgent before the agent enters the simulation.
     */
    EntityFuncSymbol *initialize_events_fn;

    /**
     * The agent function which finalizes all events in the agent.
     * 
     * This function has the fixed name om_events_tables(). It is used in the run-time support class
     * BaseAgent before the agent leaves the simulation.
     */
    EntityFuncSymbol *finalize_events_fn;

    /**
     * The agent function which initializes all entity sets in the agent.
     * 
     * This function has the fixed name om_initialize_entity_sets().  It is used in the run-time support
     * class BaseAgent before the agent enters the simulation.
     */
    EntityFuncSymbol *initialize_entity_sets_fn;

    /**
     * The agent function which finalizes all entity sets in the agent.
     * 
     * This function has the fixed name om_finalize_entity_sets(). It is used in the run-time support
     * class BaseAgent before the agent leaves the simulation.
     */
    EntityFuncSymbol *finalize_entity_sets_fn;

    /**
     * The agent function which initializes all tables in the agent.
     * 
     * This function has the fixed name om_initialize_tables().  It is used in the run-time support
     * class BaseAgent before the agent enters the simulation.
     */
    EntityFuncSymbol *initialize_tables_fn;

    /**
     * The agent function which finalizes all tables in the agent.
     * 
     * This function has the fixed name om_finalize_tables(). It is used in the run-time support
     * class BaseAgent before the agent leaves the simulation.
     */
    EntityFuncSymbol *finalize_tables_fn;

    /**
     * The agent function which initializes all identity attributes in the agent.
     * 
     * This function has the fixed name om_initialize_identity_attributes().  It is used in the run-
     * time support class BaseAgent before the agent enters the simulation.
     */
    EntityFuncSymbol *initialize_identity_attributes_fn;

    /**
     * The agent function which initializes derived attributes in the agent.
     * 
     * This function has the fixed name om_initialize_derived_attributes().  It is used in the run-
     * time support class BaseAgent to initialize values as part of the entity lifecycle.
     */
    EntityFuncSymbol *initialize_derived_attributes_fn;

    /**
     * The agent function which resets derived attributes in the agent.
     * 
     * This function has the fixed name om_reset_derived_attributes().  It is used in the run-
     * time support class BaseAgent as part of the entity lifecycle.
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
     * The expression agentvars of this agent.
     * 
     * Populated after parsing is complete.
     */
    list<IdentityAttributeSymbol *> pp_identity_agentvars;

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
     * The link agentvars of this agent
     * 
     *  Populated after parsing is complete.
     */
    list<LinkAttributeSymbol *> pp_link_agentvars;

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
     * Create event to support self-scheduling derived agentvars.
     * 
     * If not already created, create an event symbol and associated event time and event implement
     * function symbols to support self-scheduling derived agentvars.  Self-scheduling derived
     * agentvars will inject code into these functions to implement themselves.
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
     * The self-scheduling event.
     */
    EntityEventSymbol *ss_event;

    /**
     * Used to assign numeric identifiers to self-scheduling attributes in post-parse processing
     */
    int next_ss_id;
};

