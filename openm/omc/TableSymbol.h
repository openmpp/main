/**
* @file    TableSymbol.h
* Declarations for the TableSymbol class.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#pragma once
#include <list>
#include "Symbol.h"

#include "DimensionSymbol.h"
#include "MeasureDimensionSymbol.h"
#include "TableMeasureSymbol.h"

class CodeBlock;
class MeasureDimensionSymbol;


/**
* TableSymbol.
*
*/
class TableSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    /**
    * Morphing constructor.
    *
    * @param [in,out]  sym The symbol to be morphed.
    */
    TableSymbol(Symbol *sym, omc::location decl_loc = omc::location())
        : Symbol(sym, decl_loc)
        , measures_position(0)
        , is_internal(false)
        , is_suppressed(false)
        , is_hidden(false)
        , measure_dimension(nullptr)
        , pp_table_id(-1)
    {
        cxx_type = name;
        cxx_instance = "the" + name;
    }

    void post_parse(int pass);

    /** Mark enumerations required for metadata support for this table. */
    void post_parse_mark_enumerations(void);

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

    /**
     * The C++ initializer list for the shape of the table
     * 
     * Example: {2, 5} for a 2x5 table.
     * 
     * @return A string.
     */
    string cxx_shape_initializer_list() const;

    /**
     * The C++ initializer list for the dimension with a margin
     *
     * Example: {0, 2} for a table with margins for the first and third dimensions.
     *
     * @return A string.
     */
    string cxx_margin_dims_initializer_list() const;

    /**
     * The C++ initializer list for the measure names.
     * 
     * Examples:
     *   {"SIMULATED", "ESTIMATE"}
     *   {"Expr0", "Expr1", "Expr2"}
     *
     * @return A string.
     */
    string cxx_measure_name_initializer_list() const;

    void populate_metadata(openm::MetaModelHolder & metaRows);

    /**
     * The number of measures in the table.
     *
     * @return A size_t.
     */
    size_t measure_count() const
    {
        return pp_measures.size();
    }

    /**
     * The number of dimensions in the table (rank).
     *
     * @return A size_t.
     */
    size_t dimension_count() const
    {
        return dimension_list.size();
    }

    /**
     * The total number of table cells in the table.
     * 
     * Includes margin cells.
     * 
     * @return An size_t.
     */
    size_t cell_count() const;

    /**
     * The total number of dimensions with margins in the table.
     *
     * @return A size_t.
     */
    size_t margin_count() const;

    /**
     * The C++ initializer for the table
     * 
     * This is an expression list used to initialize the table at construction.
     * The expression list does not include the enclosing braces used for construction.
     * 
     * @return A string.
     */
    virtual string cxx_initializer() const = 0;

    /**
     * The dimensions in the table
     */
    list<DimensionSymbol *> dimension_list;

    /**
     * The measures in the table.
     */
    list<TableMeasureSymbol *> pp_measures;

    /**
     * The display position of measures relative to dimensions
     * 
     * The value is the zero-based ordinal of the dimension after which the measures will be
     * displayed. If the mesaures are displayed before the first dimension, the value is -1.
     */
    int measures_position;

    /**
     * Type name used to declare the table.
     */
    string cxx_type;

    /**
     * The name of the unitary global instance of the table.
     */
    string cxx_instance;

    /**
     * True if the table is internal
     * 
     * An internal table is computed during the simulation, but is not published to the DB.
     */
    bool is_internal;

    /**
     * True if the table is marked for suppression by tables_suppressed or tables_retain.
     * 
     * A suppressed table is completely removed from the model, including all generated code,
     * eg injected code in call backs, any associated attributes, etc.
     */
    bool is_suppressed;

    /**
     * True if the table is hidden
     */
    bool is_hidden;

    /**
     * Pointer to the single measure dimension of this table
     */
    MeasureDimensionSymbol * measure_dimension;

    /**
     * Other tables required by this table.
     * Comes from dependency statements in model code.
     */
    set<TableSymbol *> pp_tables_required;

    /**
     * Other tables requiring this table.
     * Comes from dependency statements in model code.
     */
    set<TableSymbol*> pp_tables_requiring;

    /**
     * Numeric identifier. Used for communicating with metadata API.
     */
    int pp_table_id;
};
