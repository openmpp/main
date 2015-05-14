/**
* @file    DerivedTableSymbol.h
* Declarations for the DerivedTableSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <list>
#include "Symbol.h"

#include "DerivedTableDimensionSymbol.h"
#include "DerivedTablePlaceholderSymbol.h"

class CodeBlock;

/**
* DerivedTableSymbol.
*
*/
class DerivedTableSymbol : public Symbol
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
    DerivedTableSymbol(Symbol *sym, yy::location decl_loc = yy::location())
        : Symbol(sym, decl_loc)
        , expr_dim_position(0)
        , pp_table_id(-1)
    {
        cxx_type = name;
        cxx_instance = "the" + name;
    }

    void post_parse(int pass);

    CodeBlock cxx_declaration_global();

    CodeBlock cxx_definition_global();

    /**
     * Create the C++ initializer list for the shape of the table
     * 
     * Example: {2, 5} for a 2x5 table.
     * 
     * @return A string.
     */
    string cxx_shape_initializer_list() const;

    /**
     * Create the C++ initializer for the table
     * 
     * This is an expression list used to initialize the table at construction.
     * The expression list does not include the enclosing parentheses used for construction.
     * 
     * Example: ({2, 5}) for a 2x5 table
     * 
     * @return A string.
     */
    string cxx_initializer() const;

    void populate_metadata(openm::MetaModelHolder & metaRows);

    /**
     * The rank of the table.
     *
     * @return An int.
     */
    int rank() const;

    /**
     * The total number of table cells in the table.
     *
     * @return An int.
     */
    int cell_count() const;

    /**
     * List of dimensions
     */
    list<DerivedTableDimensionSymbol *> dimension_list;

    /**
     * The placeholders in the derived table.
     */
    list<DerivedTablePlaceholderSymbol *> pp_placeholders;

    /**
     * The expression dimension display position.
     * 
     * The zero-based ordinal classification after which the expression dimension will be displayed.
     * If the expression dimension is the first dimension displayed, the value is -1.
     */
    int expr_dim_position;

    /**
     * Type used to declare the derived table.
     */
    string cxx_type;

    /**
     * The name of the unitary global instance of the derived table.
     */
    string cxx_instance;

    /**
     * Numeric identifier. Used for communicating with metadata API.
     */
    int pp_table_id;
};
