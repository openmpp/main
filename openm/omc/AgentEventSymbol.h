/**
* @file    AgentEventSymbol.h
* Declarations for the AgentEventSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "AgentMemberSymbol.h"

class CodeBlock;

using namespace std;

/**
* Symbol for an Agentevent
*
* An AgentEventSymbol is created when an 'event' statement is encountered
* in the model source code.  The name of the symbol
* is the name of the event implement function specified in the 'event' statement.
*/
class AgentEventSymbol : public AgentMemberSymbol
{
private:
    typedef AgentMemberSymbol super;

public:

    /**
    * Constructor.
    *
    * @param [in,out]  sym     The symbol to be morphed (event implement function)
    * @param   agent           The agent.
    * @param   time_function   The symbol for the associated event time function.
    */

    AgentEventSymbol(Symbol *sym, const Symbol *agent, const Symbol *time_function, yy::location decl_loc = yy::location())
        : AgentMemberSymbol(sym, agent, decl_loc)
        , time_function(time_function->get_rpSymbol())
    {
    }

    CodeBlock cxx_declaration_agent();

    void post_parse(int pass);

    /** The Symbol for the time function of the event.*/
    Symbol*& time_function;

    /**
    * Numeric identifier for the event.
    * The numeric identifier is the ordinal of the event name
    * among all events in the model.  If two agents have an
    * event with the same name, \a pp_event_id will be identical
    * for those two events.
    */

    int pp_event_id;
};

