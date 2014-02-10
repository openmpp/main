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
class IdentityAgentVarSymbol;
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
        , initialize_agentvar_offsets_fn(nullptr)
        , initialize_event_offsets_fn(nullptr)
        , initialize_data_members_fn(nullptr)
        , initialize_events_fn(nullptr)
        , finalize_events_fn(nullptr)
        , initialize_tables_fn(nullptr)
        , finalize_tables_fn(nullptr)
        , initialize_expression_agentvars_fn(nullptr)
    {
        create_auxiliary_symbols();
    }


    /**
     * Create auxiliary symbols associated with this agent.
     */

    void create_auxiliary_symbols();

    void post_parse(int pass);

    /**
     * Builds the function body of the function.
     */

    void build_body_initialize_agentvar_offsets();

    /**
     * Builds the function body of the function.
     */

    void build_body_initialize_event_offsets();

    /**
     * Builds the function body of the function.
     */

    void build_body_initialize_data_members();

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

    void build_body_initialize_tables();

    /**
     * Builds the function body of the function.
     */

    void build_body_finalize_tables();

    /**
     * Builds the function body of the function.
     */

    void build_body_initialize_expression_agentvars();


    /**
     * The built-in agentvar for time in the agent.
     */

    BuiltinAgentVarSymbol *pp_time;


    /**
     * The agent function which initializes the offsets of agentvars within the agent.
     * 
     * This function has the fixed name om_initialize_agentvar_offsets().  It is called in the run-time support
     * class BaseAgent exactly once for each kind of agent.
     */

    AgentFuncSymbol *initialize_agentvar_offsets_fn;


    /**
     * The agent function which initializes the offsets of events within the agent.
     * 
     * This function has the fixed name om_initialize_event_offsets().  It is called in the run-time support
     * class BaseAgent exactly once for each kind of agent.
     */

    AgentFuncSymbol *initialize_event_offsets_fn;


    /**
     * The agent function which initializes the values of all data members in the agent.
     * 
     * This function has the fixed name om_initialize_data_members().  It is called in the run-time support
     * class BaseAgent before the agent enters the simulation.
     */

    AgentFuncSymbol *initialize_data_members_fn;


    /**
     * The agent function which initializes all events in the agent.
     * 
     * This function has the fixed name om_initialize_events().  It is used in the run-time support
     * class BaseAgent before the agent enters the simulation.
     */

    AgentFuncSymbol *initialize_events_fn;


    /**
     * The agent function which finalizes all events in the agent.
     * 
     * This function has the fixed name om_events_tables(). It is used in the run-time support
     * class BaseAgent before the agent leaves the simulation.
     */

    AgentFuncSymbol *finalize_events_fn;

    /**
     * The agent function which initializes all tables in the agent.
     * 
     * This function has the fixed name om_initialize_tables().  It is used in the run-time support
     * class BaseAgent before the agent enters the simulation.
     */

    AgentFuncSymbol *initialize_tables_fn;


    /**
     * The agent function which finalizes all tables in the agent.
     * 
     * This function has the fixed name om_finalize_tables(). It is used in the run-time support
     * class BaseAgent before the agent leaves the simulation.
     */

    AgentFuncSymbol *finalize_tables_fn;


    /**
     * The agent function which initializes all expression agentvars in the agent.
     * 
     * This function has the fixed name om_initialize_expression_agentvars().  It is used in the run-time support
     * class BaseAgent before the agent enters the simulation.
     */

    AgentFuncSymbol *initialize_expression_agentvars_fn;

    /**
    * The data members of this agent
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
     * The expression agentvars of this agent.
     * 
     * Populated after parsing is complete.
     */

    list<IdentityAgentVarSymbol *> pp_expr_agentvars;


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

