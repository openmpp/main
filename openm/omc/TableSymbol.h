/**
* @file    TableSymbol.h
* Declares the TableSymbol derived class of the Symbol class.
* Also declares related classes.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <list>
#include "Symbol.h"

class TableExpressionSymbol;
class TableAccumulatorSymbol;
class TableAnalysisAgentVarSymbol;
class AgentFuncSymbol;
class AgentInternalSymbol;
class CodeBlock;

using namespace std;



/**
* TableSymbol.
*/
class TableSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    TableSymbol(Symbol *sym, const Symbol *agent, yy::location decl_loc = yy::location())
        : Symbol(sym, decl_loc)
        , agent(agent->stable_rp())
        , pp_agent(nullptr)
        , cell(nullptr)
        , update_cell_fn(nullptr)
        , prepare_increment_fn(nullptr)
        , process_increment_fn(nullptr)
    {
        create_auxiliary_symbols();
    }

    /**
     * Create auxiliary symbols associated with this table.
     */

    void create_auxiliary_symbols();

    void post_parse(int pass);

    /**
     * Gets the name of the agent member which holds the value of the active table cell
     * 
     * This is the name of the data member which holds the "in" value of the agentvar.  It holds the
     * value of the agentvar at the start of an increment, and is used to compute 'delta', etc. when
     * the increment is finalized and passed to an accumulator.
     *
     * @return The name as a string.
     */

    string cell_member_name() const;


    /**
    * Get name of member function which handles increments used in the table
    *
    * Example:  DurationOfLife_do_increments
    *
    * @return  The qualified function name as a string
    */

    const string do_increments_func();

    /**
    * Get declaration of member function which handles increments used in the table
    *
    * Example:  void Person::DurationOfLife_do_increments(int cell, bool prepare = true, bool process = true);
    */

    const string do_increments_decl();

    /**
    * Get definition of member function which handles increments used in the table
    *
    * Example:  void Person::DurationOfLife_do_increments(int cell, bool prepare = true, bool process = true)
    */

    const string do_increments_defn();

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

    CodeBlock cxx_declaration_agent();

    CodeBlock cxx_definition_agent();


    /**
     * Builds the function body of the prepare_increment function.
     */

    void build_body_prepare_increment();

    /**
     * Builds the function body of the process_increment function.
     */

    void build_body_process_increment();

    void populate_metadata(openm::MetaModelHolder & metaRows);

    /**
    * Reference to pointer to agent.
    *
    * Stable to symbol morphing during parse phase.
    */

    Symbol*& agent;


    /**
     * The agentvar which will hold the active index into the table.
     */

    AgentInternalSymbol *cell;

    /**
     * The agent function which updates the active cell index using agentvars.
     */

    AgentFuncSymbol *update_cell_fn;


    /**
     * The agent function which prepares the increment for the active table cell.
     */

    AgentFuncSymbol *prepare_increment_fn;


    /**
     * The agent function which processes the increment for the active table cell.
     */

    AgentFuncSymbol *process_increment_fn;

    /**
    * Direct pointer to agent.
    *
    * Set post-parse for convenience.
    */

    AgentSymbol *pp_agent;


    /**
    * The expressions in the table
    */

    list<TableExpressionSymbol *> pp_expressions;

    /**
    * The agentvars used in all expressions in the table
    */

    list<TableAnalysisAgentVarSymbol *> pp_table_agentvars;

    /**
    * The accumulators used in all expressions in the table
    */

    list<TableAccumulatorSymbol *> pp_accumulators;

    /**
    * Numeric identifier.
    * Used for communicating with metadata API.
    */

    int pp_table_id;
};
