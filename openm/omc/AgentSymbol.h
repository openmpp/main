/**
* @file    AgentSymbol.h
* Declares the AgentSymbol derived class of the Symbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <list>
#include "Symbol.h"

class AgentDataMemberSymbol;
class AgentEventSymbol;
class AgentEventTimeSymbol;
class AgentFuncSymbol;
class AgentVarSymbol;
class AgentInternalSymbol;
class BuiltinAgentVarSymbol;

//#include "BuiltinAgentVarSymbol.h"
//#include "AgentInternalSymbol.h"
//#include "AgentEventSymbol.h"
//#include "AgentEventTimeSymbol.h"
//#include "AgentFuncSymbol.h"


using namespace std;

/**
* AgentSymbol.
*/
class AgentSymbol : public Symbol
{
private:
    typedef Symbol super;

public:

    /**
    * Constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    AgentSymbol(Symbol *sym)
        : Symbol(sym)
        , pp_time(nullptr)
        , pp_age(nullptr)
        , pp_events(nullptr)
    {
        create_builtin_symbols();
    }

    void create_builtin_symbols();
    void post_parse(int pass);

    BuiltinAgentVarSymbol *pp_time;
    BuiltinAgentVarSymbol *pp_age;
    BuiltinAgentVarSymbol *pp_events;

    /**
    * The agentvars of this agent
    *
    *  Populated after parsing is complete.
    */

    list<AgentDataMemberSymbol *> pp_agent_data_members;

    /**
    * The agentvars of this agent
    *
    *  Populated after parsing is complete.
    */

    list<AgentVarSymbol *> pp_agentvars;

    /**
    * The agentevents of this agent
    *
    *  Populated after parsing is complete.
    */

    list<AgentEventSymbol *> pp_agentevents;

    /**
    * The AgentEventTime symbols of this agent
    *
    *  Populated after parsing is complete.
    */

    list<AgentEventTimeSymbol *> pp_agent_event_times;

    /**
    * The agentfuncs of this agent
    *
    *  Populated after parsing is complete.
    */

    list<AgentFuncSymbol *> pp_agentfuncs;

    /**
    * The tables of this agent
    *
    *  Populated after parsing is complete.
    */

    list<TableSymbol *> pp_agent_tables;
};

