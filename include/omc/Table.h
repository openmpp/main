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
 * @tparam t_cells        Number of cells.
 * @tparam t_accumulators Number of accumulators.
 * @tparam t_expressions  Number of expressions.
 */
template<size_t t_cells, size_t t_accumulators, size_t t_expressions>
class Table
{
public:
    virtual void initialize_accumulators() = 0;
    virtual void extract_accumulators() = 0;
    virtual void scale_accumulators() = 0;
    virtual void compute_expressions() = 0;

    // constants
    static const size_t n_cells = t_cells;
    static const size_t n_accumulators = t_accumulators;
    static const size_t n_expressions = t_expressions;

    // expression storage
    double expr[t_expressions][t_cells];

    // accumulator storage
    double acc[t_accumulators][t_cells];
};

/**
 * Template for tables with observation collections.
 *
 * @tparam t_cells        Number of cells.
 * @tparam t_accumulators Number of accumulators.
 * @tparam t_expressions  Number of expressions.
 * @tparam t_collections  Number of observation collections.
 */
template<size_t t_cells, size_t t_accumulators, size_t t_expressions, size_t t_collections>
class TableWithObs : public Table <t_cells, t_accumulators, t_expressions>
{
public:

    ~TableWithObs()
    {
        // Empty observation collections in all cells
        for (size_t cell = 0; cell < t_cells; ++cell) {
            for (size_t j = 0; j < t_collections; ++j) {
                coll[cell][j].clear();
            }
        }
    }

    // constants
    static const size_t n_collections = t_collections;

    // observation collection storage
    forward_list<double> coll[t_cells][t_collections];
};
