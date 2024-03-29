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
class ClassificationSymbol;

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
        , lifecycle_enter_fn(nullptr)
        , lifecycle_exit_fn(nullptr)
        , lifecycle_event_fn(nullptr)
        , start_trace_fn(nullptr)
        , ss_time_fn(nullptr)
        , ss_implement_fn(nullptr)
        , get_entity_key_fn(nullptr)
        , get_microdata_key_fn(nullptr)
        , ss_event(nullptr)
        , next_ss_id(0)
        , pp_entity_id(-1)
        , pp_local_rng_streams_requested(false)
        , pp_local_rng_streams_present(false)
        , pp_lifecycle_attributes_requested(false)
        , pp_lifecycle_classification(nullptr)
        , any_set_has_order_clause(false)
    {
        create_auxiliary_symbols();
        // Place in earliest sorting group so that missing symbols. e.g. case_id
        // are created or morphed before other symbols in first post_parse pass.
        sorting_group = 0;
    }

    /**
     * Create auxiliary symbols associated with this entity.
     */
    void create_auxiliary_symbols();

    void post_parse(int pass);

    /** Mark enumerations required for metadata support for this entity. */
    void mark_enumerations_to_publish(void);

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
     * Builds the function body of the function.
     */
    void build_body_lifecycle_event();

    /**
     * The built-in attribute for time in the entity.
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
     * The entity function which initializes the values of all data members in the entity.
     * 
     * This function has the fixed name om_initialize_data_members().  It is called in the run-time
     * support class BaseEntity before the entity enters the simulation.
     */
    EntityFuncSymbol *initialize_data_members_fn;

    /**
     * The entity function which initializes the values of all data members in the entity to type
     * default values.
     * 
     * This function has the fixed name om_initialize_data_members().  It is called in the run-time
     * support class BaseEntity before the entity enters the simulation.
     */
    EntityFuncSymbol *initialize_data_members0_fn;

    /**
     * The entity function which initializes all events in the entity.
     * 
     * This function has the fixed name om_initialize_events().  It is used in the run-time support
     * class BaseEntity before the entity enters the simulation.
     */
    EntityFuncSymbol *initialize_events_fn;

    /**
     * The entity function which finalizes all events in the entity.
     * 
     * This function has the fixed name om_events_tables(). It is used in the run-time support class
     * BaseEntity before the entity leaves the simulation.
     */
    EntityFuncSymbol *finalize_events_fn;

    /**
     * The entity function which initializes all entity sets in the entity.
     * 
     * This function has the fixed name om_initialize_entity_sets().  It is used in the run-time support
     * class BaseEntity before the entity enters the simulation.
     */
    EntityFuncSymbol *initialize_entity_sets_fn;

    /**
     * The entity function which finalizes all entity sets in the entity.
     * 
     * This function has the fixed name om_finalize_entity_sets(). It is used in the run-time support
     * class BaseEntity before the entity leaves the simulation.
     */
    EntityFuncSymbol *finalize_entity_sets_fn;

    /**
     * The entity function which initializes all tables in the entity.
     * 
     * This function has the fixed name om_initialize_tables().  It is used in the run-time support
     * class BaseEntity before the entity enters the simulation.
     */
    EntityFuncSymbol *initialize_tables_fn;

    /**
     * The entity function which finalizes all tables in the entity.
     * 
     * This function has the fixed name om_finalize_tables(). It is used in the run-time support
     * class BaseEntity before the entity leaves the simulation.
     */
    EntityFuncSymbol *finalize_tables_fn;

    /**
     * The entity function which initializes all identity attributes in the entity.
     * 
     * This function has the fixed name om_initialize_identity_attributes().  It is used in the run-
     * time support class BaseEntity before the entity enters the simulation.
     */
    EntityFuncSymbol *initialize_identity_attributes_fn;

    /**
     * The entity function which initializes derived attributes in the entity.
     * 
     * This function has the fixed name om_initialize_derived_attributes().  It is used in the run-
     * time support class BaseEntity to initialize values as part of the entity lifecycle.
     */
    EntityFuncSymbol *initialize_derived_attributes_fn;

    /**
     * The entity function which resets derived attributes in the entity.
     * 
     * This function has the fixed name om_reset_derived_attributes().  It is used in the run-
     * time support class BaseEntity as part of the entity lifecycle.
     */
    EntityFuncSymbol *reset_derived_attributes_fn;

    /**
     * The entity function which sets all links to nullptr when the entity finishes.
     */
    EntityFuncSymbol *finalize_links_fn;

    /**
     * The entity function which empties all multilinks when the entity finishes.
     */
    EntityFuncSymbol *finalize_multilinks_fn;

    /**
     * The entity function which supports lifecycle attributes on enter_simulation.
     */
    EntityFuncSymbol* lifecycle_enter_fn;

    /**
     * The entity function which supports lifecycle attributes on exit_simulation.
     */
    EntityFuncSymbol* lifecycle_exit_fn;

    /**
     * The entity function which supports lifecycle attributes at events.
     */
    EntityFuncSymbol* lifecycle_event_fn;

    /**
     * The entity function which implements trace messages at the start of the entity lifecycle.
     */
    EntityFuncSymbol* start_trace_fn;

    /**
     * The data members of this entity
     * 
     *  Populated after parsing is complete.
     */
    list<EntityDataMemberSymbol *> pp_data_members;

    /**
     * The data members of this entity which use callbacks to implement side-effects.
     * 
     *  Populated after parsing is complete.
     */
    list<EntityMemberSymbol *> pp_callback_members;

    /**
     * The maintained attributes of this entity.
     * 
     *  Populated after parsing is complete.
     */
    list<MaintainedAttributeSymbol *> pp_maintained_attributes;

    /**
     * The expression attributes of this entity.
     * 
     * Populated after parsing is complete.
     */
    list<IdentityAttributeSymbol *> pp_identity_attributes;

    /**
     * The events of this entity
     * 
     *  Populated after parsing is complete.
     */
    list<EntityEventSymbol *> pp_events;

    /**
     * The functions of this entity
     * 
     *  Populated after parsing is complete.
     */
    list<EntityFuncSymbol *> pp_functions;

    /**
     * The sets of this entity
     * 
     *  Populated after parsing is complete.
     */
    list<EntitySetSymbol *> pp_sets;

    /**
     * The tables of this entity
     * 
     *  Populated after parsing is complete.
     */
    list<EntityTableSymbol *> pp_tables;

    /**
     * The link attributes of this entity
     * 
     *  Populated after parsing is complete.
     */
    list<LinkAttributeSymbol *> pp_link_attributes;

    /**
     * The multilink members of this entity
     * 
     *  Populated after parsing is complete.
     */
    list<EntityMultilinkSymbol *> pp_multilink_members;

    /**
     * The hooks in the entity.
     * 
     * The key of the multimap is the name of the 'to' function of the hook.  The value of the multimap is the
     * name of the 'from' function.
     */
    multimap<string,string> pp_hooks;

    /**
     * The hooks in the entity, indexed by name and order
     * 
     * The key of the multimap is the name of the 'to' function of the hook, followed by the order.
     * The value of the multimap is the name of the 'from' function.
     */
    multimap<string,string> pp_hooks_with_order;

    /**
     * The RNG streams used in the entity
     *
     * Includes RNG streams used in the entity member functions,
     * e.g. calls like RandUniform(12), RandNormal(13), RandLogistic(14)
     */
    set<int> pp_rng_streams;

    /**
     * The RNG Normal streams used in the entity
     *
     * Includes RNG Normal streams used in the entity member functions,
     * e.g. calls like RandNormal(12)
     */
    set<int> pp_rng_normal_streams;

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
     * The get_entity_key function.
     */
    EntityFuncSymbol* get_entity_key_fn;

    /**
     * The get_microdata_key() function.
     */
    EntityFuncSymbol* get_microdata_key_fn;

    /**
     * The write_microdata() function.
     */
    EntityFuncSymbol* write_microdata_fn;

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
     * Indicates if local RNG streams were requested for this entity kind.
     */
    bool pp_local_rng_streams_requested;

    /**
     * Indicates if local RNG streams are present in this entity kind.
     */
    bool pp_local_rng_streams_present;

    /**
     * Indicates if lifecycle attributes were requested for this entity kind.
     */
    bool pp_lifecycle_attributes_requested;

    /**
     * The lifecycle classification, if requested
     */
    ClassificationSymbol* pp_lifecycle_classification;

    /**
     * The lifecycle map from event name to classification level name, if requested
     */
    map<string, string> pp_lifecycle_name_map;

    /**
     * Indicates if any set of this entity kind has an order clause.
     */
    bool any_set_has_order_clause;
};

