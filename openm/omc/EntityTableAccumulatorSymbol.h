/**
* @file    EntityTableAccumulatorSymbol.h
* Declarations for the EntityTableAccumulatorSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <cassert>
#include "Symbol.h"
#include "AttributeSymbol.h"
#include "EntityTableSymbol.h"

class AttributeSymbol;
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

    EntityTableAccumulatorSymbol(Symbol *table, token_type statistic, token_type increment, token_type tabop, Symbol *attribute, Symbol *analysis_attribute, int index)
        : Symbol(symbol_name(table, statistic, increment, tabop, attribute))
        , table(table->stable_rp())
        , pp_table(nullptr)
        , statistic(statistic)
        , increment(increment)
        , tabop(tabop)
        , attribute(attribute ? attribute->stable_pp() : nullptr)
        , pp_attribute(nullptr)
        , analysis_attribute(analysis_attribute ? analysis_attribute->stable_pp() : nullptr)
        , pp_analysis_attribute(nullptr)
        , updates_obs_collection(false)
        , obs_collection_index(-1)
        , updates_extrema_collections(false)
        , extrema_collections_index(-1)
        , index(index)
    {
        // grammar guarantee
        assert(statistic == token::TK_unit
            || statistic == token::TK_sum
            || statistic == token::TK_minimum
            || statistic == token::TK_maximum
            || statistic == token::TK_mean
            || statistic == token::TK_variance
            || statistic == token::TK_stdev
            || statistic == token::TK_P1
            || statistic == token::TK_P2
            || statistic == token::TK_P5
            || statistic == token::TK_P10
            || statistic == token::TK_P20
            || statistic == token::TK_P25
            || statistic == token::TK_P30
            || statistic == token::TK_P40
            || statistic == token::TK_P50
            || statistic == token::TK_P60
            || statistic == token::TK_P70
            || statistic == token::TK_P75
            || statistic == token::TK_P80
            || statistic == token::TK_P90
            || statistic == token::TK_P95
            || statistic == token::TK_P98
            || statistic == token::TK_P99
            || statistic == token::TK_gini
        );

        // grammar guarantee
        assert((increment == token::TK_unused && (statistic == token::TK_unit))
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
        assert((tabop == token::TK_unused && (statistic == token::TK_unit))
            || tabop == token::TK_interval
            || tabop == token::TK_event
            || tabop == token::TK_cell_in
            || tabop == token::TK_cell_out
        );

        // determine if there is an associated collection of observations
        has_obs_collection = 
               statistic == token::TK_gini
            || statistic == token::TK_P1
            || statistic == token::TK_P2
            || statistic == token::TK_P5
            || statistic == token::TK_P10
            || statistic == token::TK_P20
            || statistic == token::TK_P25
            || statistic == token::TK_P30
            || statistic == token::TK_P40
            || statistic == token::TK_P50
            || statistic == token::TK_P60
            || statistic == token::TK_P70
            || statistic == token::TK_P75
            || statistic == token::TK_P80
            || statistic == token::TK_P90
            || statistic == token::TK_P95
            || statistic == token::TK_P98
            || statistic == token::TK_P99
            ;
        // determine if this accumulator needs a pair of extrema collections (if table is screened)
        eligible_for_extrema_collections =
               statistic == token::TK_sum
            || statistic == token::TK_minimum
            || statistic == token::TK_maximum
            || statistic == token::TK_mean
            ;
        
    }

    /**
     * Get the unique name for this EntityTableAccumulatorSymbol.
     *
     * @param table       The table.
     * @param statistic   The statistic, e.g. token::TK_sum.
     * @param increment   The increment, e.g. token::TK_delta.
     * @param tabop      The tabop: TK_interval or TK_event.
     * @param attribute   The attribute.
     *
     * @return The name, e.g. om_ta_DurationOfLife_sum_delta_om_duration.
     */
    static string symbol_name(const Symbol *table, token_type statistic, token_type increment, token_type tabop, const Symbol *attribute);

    /**
     * Check for existence of symbol with this unique name.
     *
     * @param table       The table.
     * @param statistic   The statistic, e.g. token::TK_sum.
     * @param increment   The increment, e.g. token::TK_delta.
     * @param tabop      The tabop: TK_interval or TK_event
     * @param attribute   The attribute.
     *
     * @return true if found, else false.
     */
    static bool exists(const Symbol *table, token_type statistic, token_type increment, token_type tabop, const Symbol *attribute);

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
     * Determines if accumulator uses the value at end of the increment
     */
    bool uses_value_out() const
    {
        return increment == token::TK_delta
            || increment == token::TK_delta2
            || increment == token::TK_nz_delta
            || increment == token::TK_value_out
            || increment == token::TK_value_out2
            || increment == token::TK_nz_value_out;
    }

    /**
     * Accumulator declaration
     *
     * @param   full     (Optional) True to produce full version including defaults.
     * @param   ellipses (Optional) True to use ellipses instead of attribute name.
     *
     * @returns A string, example: value_in(age)
     */
    string declaration(bool full = false, bool ellipses = false) const
    {
        string result;
        if (statistic == token::TK_unit) {
            result = "unit";
        }
        else {
            if (ellipses) {
                result = "...";
            }
            else {
                auto a = pp_attribute;
                assert(a); // all accumulators except unit have an attribute
                result = a->pretty_name();
            }
            // surround attribute with table operator, increment, statistic if not defaults
            if (full || (tabop != token::TK_interval)) {
                result = token_to_string(tabop) + "(" + result + ")";
            }
            if (full || (increment != token::TK_delta)) {
                result = token_to_string(increment) + "(" + result + ")";
            }
            if (full || (statistic != token::TK_sum)) {
                result = token_to_string(statistic) + "(" + result + ")";
            }
        }
        return result;
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
     * The statistic of the accumulator, e.g. TK_sum, TK_minimum, TK_maximum
     */
    token_type statistic;

    /**
     * The kind of increment, e.g. TK_delta, TK_value_in
     */
    token_type increment;

    /**
     * The increment tabop: TK_interval or TK_event.
     */
    token_type tabop;

    /**
     * The attribute being accumulated (pointer to pointer)
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol** attribute;

    /**
     * The attribute being accumulated (pointer)
     * 
     * Only valid after post-parse phase 1.
     */
    AttributeSymbol* pp_attribute;

    /**
     * The analysis attribute being accumulated (pointer to pointer)
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol** analysis_attribute;

    /**
     * The analysis attribute being accumulated (pointer)
     * 
     * Only valid after post-parse phase 1.
     */
    EntityTableMeasureAttributeSymbol* pp_analysis_attribute;

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
     * Is -1 if there is no associated observation collection.
     */
    int obs_collection_index;

    /**
     * Indicates if the accumulator is eligible to have an associated pair of extrema collections of observations (if table is screened).
     */
    bool eligible_for_extrema_collections;

    /**
     * Indicates if the accumulator has an associated pair of extrema collections of observations (if table is screened).
     */
    bool has_extrema_collections() const
    {
        assert(pp_pass > eAssignMembers);
        return eligible_for_extrema_collections
            && (pp_table && pp_table->screened_extremas_size() > 0);
    }

    /**
     * Indicates if this accumulator updates its associated pair of extrema collections of observations.
     * 
     * Multiple accumulators can share a single pair of extrema collections.
     */
    bool updates_extrema_collections;

    /**
     * The zero-based index of the associated pair of extrema collections of observations (if any)
     *
     * Is -1 if there is no associated pair of extrema collections.
     */
    int extrema_collections_index;

    /**
     * Zero-based index of the accumulator within this table.
     */
    int index;
};

