/**
* @file    EntityTableSymbol.h
* Declares the EntityTableSymbol derived class of the Symbol class.
* Also declares related classes.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <list>
#include "TableSymbol.h"

class DimensionSymbol;
class EntityTableMeasureSymbol;
class EntityTableAccumulatorSymbol;
class EntityTableMeasureAttributeSymbol;
class AgentFuncSymbol;
class AgentInternalSymbol;
class AgentVarSymbol;
class IdentityAgentVarSymbol;
class BuiltinAgentVarSymbol;
class CodeBlock;

using namespace std;



/**
* EntityTableSymbol.
*/
class EntityTableSymbol : public TableSymbol
{
private:
    typedef TableSymbol super;

public:
    bool is_base_symbol() const { return false; }

    EntityTableSymbol(Symbol *sym, const Symbol *agent, yy::location decl_loc = yy::location())
        : TableSymbol(sym, decl_loc)
        , agent(agent->stable_rp())
        , pp_agent(nullptr)
        , cell(nullptr)
        , previous_global_counter(nullptr)
        , update_cell_fn(nullptr)
        , prepare_increments_fn(nullptr)
        , process_increments_fn(nullptr)
        , filter(nullptr)
        , unit(nullptr)
        , n_collections(0)
    {
        cxx_class = "cls_" + name;

        create_auxiliary_symbols();
    }

    /**
     * Create auxiliary symbols associated with this symbol.
     */
    void create_auxiliary_symbols();

    void post_parse(int pass);

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

    string cxx_initializer() const;

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
     * The number of accumulators in the table.
     *
     * @return A size_t.
     */
    size_t accumulator_count() const
    {
        return pp_accumulators.size();
    }

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
     * The expressions in the table.
     */
    //list<EntityTableMeasureSymbol *> pp_expressions;

    /**
     * The agentvars used in all expressions in the table.
     */
    list<EntityTableMeasureAttributeSymbol *> pp_table_agentvars;

    /**
     * The accumulators used in all expressions in the table.
     */
    list<EntityTableAccumulatorSymbol *> pp_accumulators;

    /**
     * The number of observation collections in the table.
     * 
     * Some accumulators such as P50 require an associated onservation collection
     * for each cell of the table.  This is the count of the number of such collections in each table cell.
     */
    int n_collections;

    /**
     * Class name used to declare the entity table.
     */
    string cxx_class;
};
