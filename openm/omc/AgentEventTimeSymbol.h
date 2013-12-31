/**
* @file    AgentEventTimeSymbol.h
* Declarations for the AgentEventTimeSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentCallbackMemberSymbol.h"

class AgentEventSymbol;

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
    AgentEventTimeSymbol(const string member_name, const Symbol *agent, const Symbol *event)
        : AgentCallbackMemberSymbol(member_name, agent, token::TK_Time)
        , event(event->get_rpSymbol())
    {
    }

    static string member_name(const Symbol *event);

    static string symbol_name(const Symbol *agent, const Symbol *event);

    void post_parse(int pass);

    /**
    * Gets the initial value for the data member
    * Events are disabled at initialization by setting the event_time to time_infinite.
    * @return  The initial value as a string.
    */

    virtual string initial_value() const
    {
        return token_to_string(token::TK_time_infinite);
    }

    /**
    * Get the c++ declaration code associated with the symbol.
    */

    virtual CodeBlock cxx_declaration_agent_scope();

    /**
    * Get the c++ definition code associated with the symbol.
    */

    virtual CodeBlock cxx_definition();

    /**
    * The associated event.
    */

    Symbol*& event;

    /**
    * The associated event.
    */

    AgentEventSymbol *pp_event;
};

