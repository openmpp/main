/**
* @file    AgentSymbol.h
* Declares the AgentSymbol derived class of the Symbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <list>
#include "Symbol.h"

class AgentDataMemberSymbol;
class AgentEventSymbol;
class AgentEventSymbol;
class AgentFuncSymbol;
class AgentVarSymbol;
class AgentInternalSymbol;
class BuiltinAgentVarSymbol;

using namespace std;

/**
* AgentSymbol.
*/
class AgentSymbol : public Symbol
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
    AgentSymbol(Symbol *sym, yy::location decl_loc = yy::location())
        : Symbol(sym, decl_loc)
        , pp_time(nullptr)
    {
        create_builtin_symbols();
    }

    void create_builtin_symbols();

    void post_parse(int pass);

    BuiltinAgentVarSymbol *pp_time;

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
    * The agent events of this agent
    *
    *  Populated after parsing is complete.
    */

    list<AgentEventSymbol *> pp_agent_events;

    /**
    * The agent funcs of this agent
    *
    *  Populated after parsing is complete.
    */

    list<AgentFuncSymbol *> pp_agent_funcs;

    /**
    * The tables of this agent
    *
    *  Populated after parsing is complete.
    */

    list<TableSymbol *> pp_agent_tables;
};

