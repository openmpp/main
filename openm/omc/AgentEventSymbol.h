/**
* @file    AgentEventSymbol.h
* Declarations for the AgentEventSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentCallbackMemberSymbol.h"
#include "NumericSymbol.h"

class AgentFuncSymbol;

using namespace std;

/**
* Event time member symbol for an event.
* Abstracts a member of an agent class which stores the event time
* of an agent event.
*/

class AgentEventSymbol : public AgentCallbackMemberSymbol
{
private:
    typedef AgentCallbackMemberSymbol super;

public:
    bool is_base_symbol() const { return false; }

    AgentEventSymbol(const string event_name, const Symbol *agent, const Symbol *time_func, const Symbol *implement_func, yy::location decl_loc = yy::location())
        : AgentCallbackMemberSymbol(event_name, agent, NumericSymbol::get_typedef_symbol(token::TK_Time), decl_loc)
        , event_name(implement_func->name)
        , time_func(time_func->get_rpSymbol())
        , implement_func(implement_func->get_rpSymbol())
        , pp_time_func(nullptr)
        , pp_implement_func(nullptr)
        , pp_event_id(0)
    {
    }

    void post_parse(int pass);


    /**
    * Name of the event.
    */

    string event_name;

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

