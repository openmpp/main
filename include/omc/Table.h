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
template<int Tdimensions, int Tcells, int Tmeasures>
class Table
{
public:
    Table(initializer_list<int> shape)
        : shape(shape)
    {
        // There are one or more measures.
        assert(n_measures > 0);

        // The rank is equal to the size of shape.
        assert(n_dimensions == shape.size());

        // The number of cells is equal to product of the values in shape.
        assert(n_cells == std::accumulate(shape.begin(), shape.end(), 1, std::multiplies<int>()));
    };

    /**
     * Initialize the measures to the undefined value (silent NaN)
     */
    void initialize_measures()
    {
        for (int cell = 0; cell < n_cells; ++cell) {
            measure[cell] = numeric_limits<double>::quiet_NaN();
        }
    };

    /**
     * Compute the flattened index corresponding to dimension indices in argument list.
     *
     * @return The index of the cell.
     */
    int get_cell_index(vector<int> indices) const
    {
        int cell = 0;
        for (int dim = 0; dim < n_dimensions; ++dim) {
            cell *= shape[dim];
            cell += indices[dim];
        }
        assert(cell < n_cells);
        return cell;
    }

    /**
     * Gets the address of a measure.
     *
     * @param measure_index The index of the meaure.
     * @param indices       The indices of the cell.
     *
     * @return null if it fails, else the measure address.
     */
    double * get_measure_address(int measure_index, vector<int> indices)
    {
        assert(measure_index < n_measures); // logic guarantee
        if (indices.size() != shape.size()) {
            //TODO run-time error Number of indices does nto match table rank
            return nullptr;
        }
        for (size_t dim = 0; dim < shape.size(); ++dim) {
            if (indices[dim] < 0 || indices[dim] >= shape[dim])
            //TODO run-time error Invalid index for dimension dim
            return nullptr;
        }
        auto cell_index = get_cell_index(indices);
        return &measure[measure_index][cell_index];
    }

    /**
     * The number of table dimensions.
     * 
     * The number of dimensions can be zero, in which case the table has no dimensions and a single
     * cell. The set of measures do not count as a dimension.
     */
    const int n_dimensions = Tdimensions;

    /**
     * The total number of cells in the table.
     */
    const int n_cells = Tcells;

    /**
     * The number of measures in the table.  A measure corresponds to an expression in a regular
     * table, and to a placeholder in a derived table.
     */
    static const int n_measures = Tmeasures;

    /**
     * The size of each dimension in the table.
     */
    const vector<int> shape;

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
template<int Tdimensions, int Tcells, int Tmeasures, int Taccumulators>
class EntityTable : public Table<Tdimensions, Tcells, Tmeasures>
{
public:
    EntityTable(initializer_list<int> shape) : Table<Tdimensions, Tcells, Tmeasures>(shape)
    {};

    virtual void initialize_accumulators() = 0;
    virtual void extract_accumulators() = 0;
    virtual void scale_accumulators() = 0;
    virtual void compute_expressions() = 0;

    static const int n_accumulators = Taccumulators;

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
template<int Tdimensions, int Tcells, int Tmeasures, int Taccumulators, int Tcollections>
class EntityTableWithObs : public EntityTable<Tdimensions, Tcells, Tmeasures, Taccumulators>
{
public:

    EntityTableWithObs(initializer_list<int> shape) : EntityTable<Tdimensions, Tcells, Tmeasures, Taccumulators>(shape)
    {
    };

    ~EntityTableWithObs()
    {
        // Empty observation collections in all cells
        for (int cell = 0; cell < Tcells; ++cell) {
            for (int j = 0; j < n_collections; ++j) {
                coll[cell][j].clear();
            }
        }
    }

    // constants
    static const int n_collections = Tcollections;

    // observation collection storage
    forward_list<double> coll[Tcells][Tcollections];
};

/**
 * Template for derived tables.
 *
 * @tparam t_cells    Number of cells.
 * @tparam t_measures Number of measures.
 */
template<int Tdimensions, int Tcells, int Tmeasures>
class DerivedTable : public Table<Tdimensions, Tcells, Tmeasures>
{
public:
    DerivedTable(initializer_list<int> shape) : Table<Tdimensions, Tcells, Tmeasures>(shape)
    {
    };
};

