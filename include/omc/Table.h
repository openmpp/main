/**
 * @file    Table.h
 * Declares templates and classes associated with tables.
 *         
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

using namespace std;

/**
 * Template for tables.
 *
 * @tparam cells          Number of cells.
 * @tparam accumulators   Number of accumulators.
 * @tparam expressions    Number of expressions.
 */
template<size_t cells, size_t accumulators, size_t expressions>
class Table
{
public:
    virtual void initialize_accumulators() = 0;
    virtual void extract_accumulators() = 0;
    virtual void scale_accumulators() = 0;
    virtual void compute_expressions() = 0;

    // constants
    static const size_t n_cells = cells;
    static const size_t n_accumulators = accumulators;
    static const size_t n_expressions = expressions;

    // expression storage
    double expr[n_expressions][n_cells];

    // accumulator storage
    double acc[n_accumulators][n_cells];
};

/**
 * Template for tables with observation collections.
 *
 * @tparam cells           Number of cells.
 * @tparam accumulators    Number of accumulators.
 * @tparam expressions     Number of expressions.
 * @tparam obs_collections Number of observation collections.
 */
template<size_t cells, size_t accumulators, size_t expressions, size_t obs_collections>
class TableWithObs : public Table <cells, accumulators, expressions>
{
public:

    ~TableWithObs()
    {
        // Empty observation collections in all cells
        for (size_t cell = 0; cell < n_cells; ++cell) {
            for (size_t coll = 0; coll < n_obs_collections; ++coll) {
                obs_collections[cell][coll].clear();
            }
        }
    }

    // constants
    static const size_t n_obs_collections = obs_collections;

    // observation collection storage
    forward_list<double> obs_collections[n_cells][n_obs_collections];
};
