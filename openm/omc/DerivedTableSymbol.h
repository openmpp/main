/**
* @file    DerivedTableSymbol.h
* Declarations for the DerivedTableSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Symbol.h"
#include "DerivedTableDimensionSymbol.h"
#include "DerivedTablePlaceholderSymbol.h"

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
    }

    void post_parse(int pass);

    void populate_metadata(openm::MetaModelHolder & metaRows);

    /**
     * The rank of the table.
     *
     * @return An int.
     */
    int rank();

    /**
     * The total number of table cells in the table.
     *
     * @return An int.
     */
    int cell_count();

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
     * Numeric identifier. Used for communicating with metadata API.
     */
    int pp_table_id;
};
