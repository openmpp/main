/**
* @file    EntityTableAccumulatorSymbol.h
* Declarations for the EntityTableAccumulatorSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <cassert>
#include "Symbol.h"

class AgentVarSymbol;
class EntityTableMeasureAttributeSymbol;
class EntityTableSymbol;

using namespace std;

/**
* EntityTableAccumulatorSymbol.
*/
class EntityTableAccumulatorSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    EntityTableAccumulatorSymbol(Symbol *table, token_type accumulator, token_type increment, token_type table_op, Symbol *agentvar, Symbol *analysis_agentvar, int index)
        : Symbol(symbol_name(table, accumulator, increment, table_op, agentvar))
        , table(table->stable_rp())
        , pp_table(nullptr)
        , accumulator(accumulator)
        , increment(increment)
        , table_op(table_op)
        , agentvar(agentvar ? agentvar->stable_pp() : nullptr)
        , pp_agentvar(nullptr)
        , analysis_agentvar(analysis_agentvar ? analysis_agentvar->stable_pp() : nullptr)
        , pp_analysis_agentvar(nullptr)
        , updates_obs_collection(false)
        , obs_collection_index(-1)
        , index(index)
    {
        // grammar guarantee
        assert(accumulator == token::TK_unit
            || accumulator == token::TK_sum
            || accumulator == token::TK_minimum
            || accumulator == token::TK_maximum
            || accumulator == token::TK_gini
            || accumulator == token::TK_P1
            || accumulator == token::TK_P2
            || accumulator == token::TK_P5
            || accumulator == token::TK_P10
            || accumulator == token::TK_P20
            || accumulator == token::TK_P25
            || accumulator == token::TK_P30
            || accumulator == token::TK_P40
            || accumulator == token::TK_P50
            || accumulator == token::TK_P60
            || accumulator == token::TK_P70
            || accumulator == token::TK_P75
            || accumulator == token::TK_P80
            || accumulator == token::TK_P90
            || accumulator == token::TK_P95
            || accumulator == token::TK_P98
            || accumulator == token::TK_P99
            );

        // grammar guarantee
        assert(increment == token::TK_unused && accumulator == token::TK_unit
            || increment == token::TK_delta
            || increment == token::TK_delta2
            || increment == token::TK_nz_delta
            || increment == token::TK_value_in
            || increment == token::TK_value_in2
            || increment == token::TK_nz_value_in
            || increment == token::TK_value_out
            || increment == token::TK_value_out2
            || increment == token::TK_nz_value_out
            );

        // grammar guarantee
        assert(table_op == token::TK_unused && accumulator == token::TK_unit
            || table_op == token::TK_interval
            || table_op == token::TK_event
            );

        // determine if there is an associated collection of observations
        has_obs_collection = accumulator == token::TK_gini
                          || accumulator == token::TK_P1
                          || accumulator == token::TK_P2
                          || accumulator == token::TK_P5
                          || accumulator == token::TK_P10
                          || accumulator == token::TK_P20
                          || accumulator == token::TK_P25
                          || accumulator == token::TK_P30
                          || accumulator == token::TK_P40
                          || accumulator == token::TK_P50
                          || accumulator == token::TK_P60
                          || accumulator == token::TK_P70
                          || accumulator == token::TK_P75
                          || accumulator == token::TK_P80
                          || accumulator == token::TK_P90
                          || accumulator == token::TK_P95
                          || accumulator == token::TK_P98
                          || accumulator == token::TK_P99
                          ;
         
    }

    /**
     * Get the unique name for this EntityTableAccumulatorSymbol.
     *
     * @param table       The table.
     * @param accumulator The accumulator, e.g. token::TK_sum.
     * @param increment   The increment, e.g. token::TK_delta.
     * @param table_op    The table operation: TK_interval or TK_event.
     * @param agentvar    The agentvar.
     *
     * @return The name, e.g. om_ta_DurationOfLife_sum_delta_om_duration.
     */
    static string symbol_name(const Symbol *table, token_type accumulator, token_type increment, token_type table_op, const Symbol *agentvar);

    /**
     * Check for existence of symbol with this unique name.
     *
     * @param table       The table.
     * @param accumulator The accumulator, e.g. token::TK_sum.
     * @param increment   The increment, e.g. token::TK_delta.
     * @param table_op    The table operation: TK_interval or TK_event
     * @param agentvar    The agentvar.
     *
     * @return true if found, else false.
     */
    static bool exists(const Symbol *table, token_type accumulator, token_type increment, token_type table_op, const Symbol *agentvar);

    void post_parse(int pass);

    virtual string pretty_name() const;

    /**
     * Determines if accumulator uses the value at start of the increment
     */
    bool uses_value_in() const
    {
        return increment == token::TK_delta
            || increment == token::TK_delta2
            || increment == token::TK_nz_delta
            || increment == token::TK_value_in
            || increment == token::TK_value_in2
            || increment == token::TK_nz_value_in;
    }

    /**
     * The table containing this accumulator (reference to pointer)
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol*& table;

    /**
     * The table containing this accumulator (pointer)
     * 
     * Only valid after post-parse phase 1.
     */
    EntityTableSymbol* pp_table;

    /**
     * The kind of accumulator, e.g. TK_sum, TK_minimum, TK_maximum
     */
    token_type accumulator;

    /**
     * The kind of increment, e.g. TK_delta, TK_value_in
     */
    token_type increment;

    /**
     * The table operator: TK_interval or TK_event.
     */
    token_type table_op;

    /**
     * The agentvar being accumulated (pointer to pointer)
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol** agentvar;

    /**
     * The agentvar being accumulated (pointer)
     * 
     * Only valid after post-parse phase 1.
     */
    AgentVarSymbol* pp_agentvar;

    /**
     * The analysis agentvar being accumulated (pointer to pointer)
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol** analysis_agentvar;

    /**
     * The analysis agentvar being accumulated (pointer)
     * 
     * Only valid after post-parse phase 1.
     */
    EntityTableMeasureAttributeSymbol* pp_analysis_agentvar;

    /**
     * Indicates if the accumulator has an associated collection of observations.
     * 
     * Accumulators such as P50 have associated collections of increments
     */
    bool has_obs_collection;

    /**
     * Indicates if this accumulator updates its collection of observations.
     * 
     * Multiple accumulators can share a single collection of observations, e.g. P50, P25. This flag
     * is used to ensure that only one such updates the shared collection.
     */
    bool updates_obs_collection;

    /**
     * The zero-based index of the associated collection of observations (if any)
     * 
     * Set to -1 if there is no associated collection.
     */
    int obs_collection_index;

    /**
     * Zero-based index of the accumulator within this table.
     */
    int index;
};

