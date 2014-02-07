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

    AgentEventSymbol(const string evt_name, const Symbol *agent, Symbol *tfs, Symbol *ifs, yy::location decl_loc = yy::location())
        : AgentDataMemberSymbol(evt_name, agent, NumericSymbol::find(token::TK_Time), decl_loc)
        , event_name(ifs->name)
        , pp_event_id(0)
    {
        // Create an AgentFuncSymbol for the time function ('true' means the definition is developer-supplied)
        time_func = new AgentFuncSymbol(tfs, agent, "Time", "", true, decl_loc);
        time_func->doc_block = doxygen_short("Return the time to the event " + event_name + " in the " + agent->name + " agent (model code).");

        // Create an AgentFuncSymbol for the implement function ('true' means the definition is developer-supplied)
        implement_func = new AgentFuncSymbol(ifs, agent, "void", "", true, decl_loc);
        implement_func->doc_block = doxygen_short("Implement the event " + event_name + " when it occurs in the " + agent->name + " agent (model code).");
    }

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
    * @return The initial value as a string.
    */

    string initial_value() const
    {
        return token_to_string(token::TK_time_infinite);
    }

    CodeBlock cxx_declaration_agent();

    /** The time function of the event.*/
    AgentFuncSymbol *time_func;

    /** The implement function of the event.*/
    AgentFuncSymbol *implement_func;

    /**
    * Numeric identifier for the event. The numeric identifier is the ordinal of the event name
    * among all events in the model.  If two agents have an event with the same name, \a
    * pp_event_id will be identical for those two events.
    */

    int pp_event_id;
};

