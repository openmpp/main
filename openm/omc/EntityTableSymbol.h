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
        , cell_in(nullptr)
        , active(nullptr)
        , pending(nullptr)
        , pending_event_counter(nullptr)
        , update_cell_fn(nullptr)
        , start_increment_fn(nullptr)
        , finish_increment_fn(nullptr)
        , start_pending_fn(nullptr)
        , finish_pending_fn(nullptr)
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
     * Builds the function body of the update cell function.
     */
    void build_body_update_cell();

    /**
     * Builds the function body of the start increment function.
     */
    void build_body_start_increment();

    /**
     * Builds the function body of the finish increment function.
     */
    void build_body_finish_increment();

    /**
     * Builds the function body of the start pending function.
     */
    void build_body_start_pending();

    /**
     * Builds the function body of the finish pending function.
     */
    void build_body_finish_pending();

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
     * The attribute containing the active index in the table.
     */
    AgentInternalSymbol *cell;

    /**
     * The attribute containing the index in the table at increment start
     */
    AgentInternalSymbol *cell_in;

    /**
     * The attribute containing active increment status
     */
    AgentInternalSymbol *active;

    /**
     * The attribute containing pending increment status
     */
    AgentInternalSymbol *pending;

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
    AgentInternalSymbol *pending_event_counter;

    /**
     * The entity function which updates the active cell index using attributes.
     */
    AgentFuncSymbol *update_cell_fn;

    /**
     * The entity function which starts a new increment.
     */
    AgentFuncSymbol *start_increment_fn;

    /**
     * The entity function which finishes the current increment.
     */
    AgentFuncSymbol *finish_increment_fn;

    /**
     * The entity function which checks for and starts a pending increment.
     */
    AgentFuncSymbol *start_pending_fn;

    /**
     * The entity function which checks for and finishes a pending increment.
     */
    AgentFuncSymbol *finish_pending_fn;

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
     * The measure attributes used in table measures.
     */
    list<EntityTableMeasureAttributeSymbol *> pp_measure_attributes;

    /**
     * The accumulators used in table measures.
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
