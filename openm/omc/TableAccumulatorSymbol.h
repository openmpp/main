/**
* @file    TableAccumulatorSymbol.h
* Declarations for the TableAccumulatorSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"

class AgentVarSymbol;
class TableAnalysisAgentVarSymbol;
class TableSymbol;

using namespace std;

/**
* TableAccumulatorSymbol.
*/
class TableAccumulatorSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    TableAccumulatorSymbol(Symbol *table, token_type accumulator, token_type increment, Symbol *agentvar, Symbol *analysis_agentvar, int index)
        : Symbol(symbol_name(table, accumulator, increment, agentvar))
        , table(table->get_rpSymbol())
        , accumulator(accumulator)
        , increment(increment)
        , agentvar(agentvar->get_rpSymbol())
        , analysis_agentvar(analysis_agentvar->get_rpSymbol())
        , index(index)
        , pp_table(nullptr)
        , pp_agentvar(nullptr)
        , pp_analysis_agentvar(nullptr)
    {
        // grammar guarantee
        assert(accumulator == token::TK_sum
            || accumulator == token::TK_min
            || accumulator == token::TK_max
            );

        // grammar guarantee
        assert(increment == token::TK_delta
            || increment == token::TK_delta2
            || increment == token::TK_nz_delta
            || increment == token::TK_value_in
            || increment == token::TK_value_in2
            || increment == token::TK_nz_value_in
            || increment == token::TK_value_out
            || increment == token::TK_value_out2
            || increment == token::TK_nz_value_out
            );
    }

    /**
    * Get the unique name for this TableAccumulatorSymbol.
    *
    * @param   table       The table.
    * @param   accumulator The accumulator, e.g. token::TK_sum
    * @param   increment   The increment, e.g. token::TK_delta
    * @param   agentvar    The agentvar.
    *
    * @return  The name, e.g. om_ta_DurationOfLife_sum_delta_om_duration.
    */

    static string symbol_name(const Symbol *table, token_type accumulator, token_type increment, const Symbol *agentvar);

    /**
    * Check for existence of symbol with this unique name.
    *
    * @param   table       The table.
    * @param   accumulator The accumulator, e.g. token::TK_sum.
    * @param   increment   The increment, e.g. token::TK_delta.
    * @param   agentvar    The agentvar.
    *
    * @return  true if found, else false.
    */

    static bool exists(const Symbol *table, token_type accumulator, token_type increment, const Symbol *agentvar);

    void post_parse(int pass);

    virtual string pretty_name();

    /**
    * The table containing this accumulator (reference to pointer)
    *
    * Stable to symbol morphing during parse phase.
    */

    Symbol*& table;

    token_type accumulator;

    token_type increment;

    /**
    * The agentvar being accumulated (reference to pointer)
    *
    * Stable to symbol morphing during parse phase.
    */

    Symbol*& agentvar;

    /**
    * The analysis agentvar being accumulated (reference to pointer)
    *
    * Stable to symbol morphing during parse phase.
    */

    Symbol*& analysis_agentvar;

    /**
    * Zero-based index of the accumulator within this table.
    */

    int index;

    /**
    * The table containing this accumulator (pointer)
    *
    * Only valid after post-parse phase 1.
    */

    TableSymbol* pp_table;

    /**
    * The agentvar being accumulated (pointer)
    *
    * Only valid after post-parse phase 1.
    */

    AgentVarSymbol* pp_agentvar;

    /**
    * The analysis agentvar being accumulated (pointer)
    *
    * Only valid after post-parse phase 1.
    */

    TableAnalysisAgentVarSymbol* pp_analysis_agentvar;
};

