/**
 * @file    Table.h
 * Declares templates and classes associated with tables.
 *         
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <vector>
#include <algorithm>
#include <cstdarg>

using namespace std;

template<size_t Tmeasures, size_t Trank, size_t Tcells>
class BaseTable
{
public:
    BaseTable(initializer_list<size_t> shape)
        : shape(shape)
    {
        // There are one or more measures.
        assert(measures > 0);

        // The rank is equal to the size of shape.
        assert(rank == shape.size());

        // The number of cells is equal to product of the values in shape.
        assert(cells == std::accumulate(shape.begin(), shape.end(), 1, std::multiplies<size_t>()));
    };

    /**
     * Initializes the measures.
     */
    void initialize_measures()
    {
        for (size_t cell = 0; cell < n_cells; ++cell) {
            measure[cell] = UNDEF_VALUE;
        }
    };

    /**
     * Compute the flattened index corresponding to dimension indices in argument list.
     *
     * @return The index of the cell.
     */
    size_t get_cell_index(va_list argp) const
    {
        size_t cell = 0;
        va_list args;
        va_start(args, rank);
        cell = va_arg(args, size_t);
        for (size_t dim = 1; dim < rank; ++dim) {
            cell *= shape[dim];
            cell += va_args(args, size_t);
        }
        va_end(args);
        return cell;
    }

    void set_cell(...)
    {
        va_list args;
    }

    /**
     * The number of table dimensions.
     * 
     * The rank can be zero, in which case the table has no dimensions and a single cell. The set of
     * measures do not count as a dimension.
     */
    const size_t rank = Trank;

    /**
     * The total number of cells in the table.
     */
    const size_t cells = Tcells;

    /**
     * The number of measures in the table.  A measure corresponds to an expression in a regular
     * table, and to a placeholder in a derived table.
     */
    static const size_t measures = Tmeasures;

    /**
     * The size of each dimension in the table.
     */
    const vector<size_t> shape;

    /**
     * Storage for all measures in all cells in the table.
     */
    double measure[Tmeasures][Tcells];
};

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

/**
 * Template for derived tables.
 *
 * @tparam t_cells        Number of cells.
 * @tparam t_placeholders Number of placeholders.
 */
template<size_t t_cells, size_t t_placeholders>
class DerivedTable
{
public:
    // constants
    static const size_t n_cells = t_cells;
    static const size_t n_placeholders = t_placeholders;

    // placeholder storage
    double placeholder[t_placeholders][t_cells];
};

