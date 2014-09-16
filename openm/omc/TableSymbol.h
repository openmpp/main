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
class AgentVarSymbol;
class IdentityAgentVarSymbol;
class BuiltinAgentVarSymbol;
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
        , previous_global_counter(nullptr)
        , update_cell_fn(nullptr)
        , prepare_increments_fn(nullptr)
        , process_increments_fn(nullptr)
        , filter(nullptr)
        , unit(nullptr)
        , expr_dim_position(0)
        , pp_table_id(-1)
    {
        create_auxiliary_symbols();
    }

    /**
     * Create auxiliary symbols associated with this symbol.
     */
    void create_auxiliary_symbols();

    void post_parse(int pass);

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

    /**
     * Builds the function body of the update_cell function.
     */
    void build_body_update_cell();

    /**
     * Builds the function body of the prepare_increment function.
     */
    void build_body_prepare_increments();

    /**
     * Builds the function body of the process_increment function.
     */
    void build_body_process_increments();

    void populate_metadata(openm::MetaModelHolder & metaRows);

    /**
     * The rank of the table.
     *
     * @return An int.
     */
    int rank();

    /**
     * The total number of table cells in the table.
     *
     * @return An int.
     */
    int cell_count();

    /**
     * Reference to pointer to agent.
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol*& agent;

    /**
     * Direct pointer to agent.
     * 
     * For use post-parse.
     */
    AgentSymbol *pp_agent;

    /**
     * The agentvar which will hold the active index into the table.
     */
    AgentInternalSymbol *cell;

    /**
     * The agentvar which will hold the value of the global event counter at the previous table
     * update.
     * 
     * Table updates are lagged so that simultaneous changes in multiple classificatory dimensions
     * and/or filter are processed together as a single table update.  This agentvar together with
     * the current value of the global event counter is used to detect that the previous table
     * update is definitively finished and can be processed.
     * 
     * Because the global event counter is 1 for the first event, the default initialization value
     * of zero for this agentvar works as intended.
     */
    AgentInternalSymbol *previous_global_counter;

    /**
     * The agent function which updates the active cell index using agentvars.
     */
    AgentFuncSymbol *update_cell_fn;

    /**
     * The agent function which prepares the increment for the active table cell.
     */
    AgentFuncSymbol *prepare_increments_fn;

    /**
     * The agent function which processes the increment for the active table cell.
     */
    AgentFuncSymbol *process_increments_fn;

    /**
     * The expression agentvar of the table filter.
     */
    IdentityAgentVarSymbol *filter;

    /**
     * The builtin agentvar of the table 'unit'.
     */
    BuiltinAgentVarSymbol *unit;

    /**
     * List of dimensions (parse phase references to pointers)
     */
    list<Symbol **> dimension_list;

    /**
     * List of margin indicators
     */
    list<bool> margin_list;

    /**
     * List of dimensions (post-parse phase pointers)
     */
    list<AgentVarSymbol *> pp_dimension_list_agentvar;

    /**
     * List of dimensions (post-parse phase pointers)
     */
    list<EnumerationSymbol *> pp_dimension_list_enum;

    /**
     * The expressions in the table.
     */
    list<TableExpressionSymbol *> pp_expressions;

    /**
     * The agentvars used in all expressions in the table.
     */
    list<TableAnalysisAgentVarSymbol *> pp_table_agentvars;

    /**
     * The accumulators used in all expressions in the table.
     */
    list<TableAccumulatorSymbol *> pp_accumulators;

    /**
     * The expression dimension display position.
     * 
     * The zero-based ordinal classification after which the expression dimension will be displayed.
     * If the expression dimension is the first dimension displayed, the value is -1.
     */
    int expr_dim_position;

    /**
     * Numeric identifier. Used for communicating with metadata API.
     */
    int pp_table_id;
};
