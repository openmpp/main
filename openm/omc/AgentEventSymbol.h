/**
* @file    AgentEventSymbol.h
* Declarations for the AgentEventSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentDataMemberSymbol.h"
#include "AgentFuncSymbol.h"
#include "NumericSymbol.h"
#include "omc/event_priorities.h" // for access to event priority constants by modules including this header

class AgentFuncSymbol;

using namespace std;

/**
* Event time member symbol for an event.
* Abstracts a member of an agent class which stores the event time
* of an agent event.
*/

class AgentEventSymbol : public AgentDataMemberSymbol
{
private:
    typedef AgentDataMemberSymbol super;

public:
    bool is_base_symbol() const { return false; }

    AgentEventSymbol(const string evt_name, const Symbol *agent, Symbol *tfs, Symbol *ifs, bool is_developer_supplied, int event_priority, yy::location decl_loc = yy::location())
        : AgentDataMemberSymbol(evt_name, agent, NumericSymbol::find(token::TK_Time), decl_loc)
        , event_name(ifs->name)
        , event_priority(event_priority)
        , is_developer_supplied(is_developer_supplied)
        , pp_event_id(0)
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
     * AgentEventSymbol is different - it has a om_time_ prefix.  That's also the name of the agent
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
    AgentFuncSymbol *time_func;

    /**
     * The time function of the event (model code).
     *
     * A cover version of the time function is created for event logging if event_trace is activated.
     * This member retains the original function in model code for dependency analysis.
     */
    AgentFuncSymbol *time_func_model_code;

    /** The implement function of the event.*/
    AgentFuncSymbol *implement_func;

    /**
     * The event priority.
     * 
     * Used to break ties if two events are scheduled to occur at the same time.
     */
    const int event_priority;

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
};

