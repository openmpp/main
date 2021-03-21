/**
* @file    EntityEventSymbol.h
* Declarations for the EntityEventSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include "EntityDataMemberSymbol.h"
#include "EntityFuncSymbol.h"
#include "NumericSymbol.h"
#include "omc/event_priorities.h" // for access to event priority constants by modules including this header

class EntityFuncSymbol;

using namespace std;

/**
* Event time member symbol for an event.
* Abstracts a member of an agent class which stores the event time
* of an agent event.
*/

class EntityEventSymbol : public EntityDataMemberSymbol
{
private:
    typedef EntityDataMemberSymbol super;

public:
    bool is_base_symbol() const { return false; }

    EntityEventSymbol(const string evt_name, const Symbol *agent, Symbol *tfs, Symbol *ifs, bool is_developer_supplied, int event_priority, yy::location decl_loc = yy::location())
        : EntityDataMemberSymbol(evt_name, agent, NumericSymbol::find(token::TK_Time), decl_loc)
        , event_name(ifs->name)
        , event_priority(event_priority)
        , event_memory(false)
        , is_developer_supplied(is_developer_supplied)
        , pp_event_id(0)
        , pp_modgen_event_num(0)
    {
        create_auxiliary_symbols(tfs, ifs, is_developer_supplied);
    }

    /**
     * Create auxiliary symbols associated with this agent event.
     *
     * @param tfs The time function symbol.
     * @param ifs The implement function symbol.
     */
    void create_auxiliary_symbols(Symbol *tfs, Symbol *ifs, bool is_developer_supplied);

    void post_parse(int pass);

    /**
     * Name of the event.
     * 
     * event_name is actually the same as the name of the event implement function.  The name of the
     * EntityEventSymbol is different - it has a om_time_ prefix.  That's also the name of the agent
     * member which holds the time of the event.
     */
    string event_name;

    /**
     * Gets the initial value for the data member
     * 
     * Events are disabled at initialization by setting the event_time to time_infinite.
     *
     * @param type_default true to type default.
     *
     * @return The initial value as a string.
     */
    string initialization_value(bool type_default) const
    {
        return token_to_string(token::TK_time_infinite);
    }

    CodeBlock cxx_declaration_agent();

    /** The time function of the event.*/
    EntityFuncSymbol *time_func;

    /**
     * The time function of the event (original).
     *
     * A cover version of the time function is created for event logging if event_trace is activated.
     * This member contains the original function.
     */
    EntityFuncSymbol *time_func_original;

    /** The implement function of the event.*/
    EntityFuncSymbol *implement_func;

    /**
     * The implement function of the event (original).
     *
     * A cover version of the implement function is created for event logging if event_trace is activated.
     * This member contains the original function.
     */
    EntityFuncSymbol *implement_func_original;

    /**
     * The event priority.
     * 
     * Used to break ties if two events are scheduled to occur at the same time.
     */
    const int event_priority;

    /**
     * Event memory indicator.
     * 
     * If true, the event has auxiliary information stored as an int.
     * If true, prototypes for the event time and implement functions have specific arguments.
     */
    bool event_memory;

    /**
     * true if this object is developer-supplied, rather than generated (self-scheduling event)
     */
    bool is_developer_supplied;

    /**
     * Numeric identifier for the event. The numeric identifier is the ordinal of the event name
     * among all events in the model.  If two agents have an event with the same name, \a
     * pp_event_id will be identical for those two events.  The event id participates in
     * resolution of tied events, after event time and event priority.
     */
    int pp_event_id;

    /**
     * Modgen-compatible numeric identifier for the event. The numeric identifier is the ordinal of
     * the event name among all events in the entity, ignoring the schedule event.  This identifier
     * is used to generate event checksums compatible with Modgen, to allow exact checksum
     * comparisons.
     */
    int pp_modgen_event_num;
};

