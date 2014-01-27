/**
* @file    AgentEventTimeSymbol.h
* Declarations for the AgentEventTimeSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentCallbackMemberSymbol.h"
#include "TypedefTypeSymbol.h"
#include "AgentSymbol.h"  // CAN BE REMOVED

class AgentEventSymbol;
class AgentFuncSymbol;

using namespace std;

/**
* Event time member symbol for an event.
* Abstracts a member of an agent class which stores the event time
* of an agent event.
*/

class AgentEventTimeSymbol : public AgentCallbackMemberSymbol
{
private:
    typedef AgentCallbackMemberSymbol super;

public:
    bool is_base_symbol() const { return false; }

    AgentEventTimeSymbol(const string event_name, const Symbol *agent, const Symbol *time_func, const Symbol *implement_func, yy::location decl_loc = yy::location())
        : AgentCallbackMemberSymbol(event_name, agent, TypedefTypeSymbol::get_typedef_symbol(token::TK_Time), decl_loc)
        , time_func(time_func->get_rpSymbol())
        , implement_func(implement_func->get_rpSymbol())
        , pp_time_func(nullptr)
        , pp_implement_func(nullptr)
        , pp_event_id(0)
    {
    }

    void post_parse(int pass);

    /** The name of the Event<> template symbol for this event */
    string event_member_name()
    {
        return "om_time_" + event_name;
    }

    /** The name of the event CHANGE TO name */
    string event_name;

    /** The name of the event CHANGE TO unique_name */
    string event_unique_name()
    {
        return pp_agent->name + "::" + event_name;
    }

    /**
     * Gets the initial value for the data member
     * 
     * Events are disabled at initialization by setting the event_time to time_infinite.
     *
     * @return The initial value as a string.
     */

    string initial_value() const
    {
        return token_to_string(token::TK_time_infinite);
    }

    CodeBlock cxx_declaration_agent();

    /** The Symbol for the time function of the event.*/
    Symbol*& time_func;

    Symbol*& implement_func;

    AgentFuncSymbol *pp_time_func;

    AgentFuncSymbol *pp_implement_func;

    /**
    * Numeric identifier for the event. The numeric identifier is the ordinal of the event name
    * among all events in the model.  If two agents have an event with the same name, \a
    * pp_event_id will be identical for those two events.
    */

    int pp_event_id;
};

