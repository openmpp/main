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

/**
 * A table.
 *
 * @tparam Tdimensions Number of dimensions.
 * @tparam Tcells      Number of cells.
 * @tparam Tmeasures   Number of measures.
 */
template<size_t Tdimensions, size_t Tcells, size_t Tmeasures>
class Table
{
public:
    Table(initializer_list<size_t> shape)
        : shape(shape)
    {
        // There are one or more measures.
        assert(n_measures > 0);

        // The rank is equal to the size of shape.
        assert(n_dimensions == shape.size());

        // The number of cells is equal to product of the values in shape.
        assert(n_cells == std::accumulate(shape.begin(), shape.end(), 1, std::multiplies<size_t>()));
    };

    /**
     * Initialize the measures to the undefined value (silent NaN)
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
        // TODO
    }

    /**
     * The number of table dimensions.
     * 
     * The number of dimensions can be zero, in which case the table has no dimensions and a single
     * cell. The set of measures do not count as a dimension.
     */
    const size_t n_dimensions = Tdimensions;

    /**
     * The total number of cells in the table.
     */
    const size_t n_cells = Tcells;

    /**
     * The number of measures in the table.  A measure corresponds to an expression in a regular
     * table, and to a placeholder in a derived table.
     */
    static const size_t n_measures = Tmeasures;

    /**
     * The size of each dimension in the table.
     */
    const vector<size_t> shape;

    /**
     * Measure storage.
     */
    double measure[Tmeasures][Tcells];
};

/**
 * Template for entity tables.
 *
 * @tparam Tdimensions Number of dimensions.
 * @tparam Tcells      Number of cells.
 * @tparam Tmeasures   Number of measures.
 * @tparam Taccumulators Number of accumulators.
 */
template<size_t Tdimensions, size_t Tcells, size_t Tmeasures, size_t Taccumulators>
class EntityTable : public Table<Tdimensions, Tcells, Tmeasures>
{
public:
    EntityTable(initializer_list<size_t> shape) : Table<Tdimensions, Tcells, Tmeasures>(shape)
    {
    };

    virtual void initialize_accumulators() = 0;
    virtual void extract_accumulators() = 0;
    virtual void scale_accumulators() = 0;
    virtual void compute_expressions() = 0;

    static const size_t n_accumulators = Taccumulators;

    /**
     * Accumulator storage.
     */
    double acc[Taccumulators][Tcells];
};

/**
 * Template for tables with observation collections.
 *
 * @tparam Tdimensions Number of dimensions.
 * @tparam Tcells      Number of cells.
 * @tparam Tmeasures   Number of measures.
 * @tparam Taccumulators Number of accumulators.
 * @tparam Tcollections  Number of observation collections.
 */
template<size_t Tdimensions, size_t Tcells, size_t Tmeasures, size_t Taccumulators, size_t Tcollections>
class EntityTableWithObs : public EntityTable<Tdimensions, Tcells, Tmeasures, Taccumulators>
{
public:

    EntityTableWithObs(initializer_list<size_t> shape) : EntityTable<Tdimensions, Tcells, Tmeasures, Taccumulators>(shape)
    {
    };

    ~EntityTableWithObs()
    {
        // Empty observation collections in all cells
        for (size_t cell = 0; cell < n_cells; ++cell) {
            for (size_t j = 0; j < n_collections; ++j) {
                coll[cell][j].clear();
            }
        }
    }

    // constants
    static const size_t n_collections = Tcollections;

    // observation collection storage
    forward_list<double> coll[Tcells][Tcollections];
};

/**
 * Template for derived tables.
 *
 * @tparam t_cells    Number of cells.
 * @tparam t_measures Number of measures.
 */
template<size_t Tdimensions, size_t Tcells, size_t Tmeasures>
class DerivedTable : public Table<Tdimensions, Tcells, Tmeasures>
{
public:
    DerivedTable(initializer_list<size_t> shape) : Table(shape)
    {
    };
};

