/**
* @file    DimensionSymbol.h
* Declarations for the DimensionSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include "Symbol.h"

class AgentVarSymbol;
class EunumerationSymbol;

using namespace std;

/**
* DimensionSymbol.
* 
* Corresponds to a dimension of a table (excluding the expression dimension of hte table).
*/
class DimensionSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    DimensionSymbol(Symbol *table_or_entity_set, int index, bool after_analysis_dim, Symbol *attribute, bool has_margin, yy::location decl_loc = yy::location())
        : Symbol(symbol_name(table_or_entity_set, index, after_analysis_dim), decl_loc)
        , index(index)
        , after_analysis_dim(after_analysis_dim)
        , has_margin(has_margin)
        , attribute(attribute->stable_rp())
        , pp_attribute(nullptr)
    {
    }

    /**
     * Construct symbol name for the dimension symbol. Example: BasicDemography.Dim0.
     *
     * @param table_or_entity_set The table or entity set containing the this dimension.
     * @param index          0-based index of the enumeration dimension among all enumeration dimensions in teh table.
     *
     * @return A string.
     */
    static string symbol_name(const Symbol* table_or_entity_set, int index, bool after_analysis_dim);

    void post_parse(int pass);

    /**
     * true if this dimension has a margin (using '+')
     */
    bool has_margin;

    /**
     * Zero-based index of the dimension in the table.
     * 
     * Note that index in DimensionSymbol does not count the special analysis dimension.
     */
    int index;

    /**
     * In a table, true if this enumeration dimension comes after ths special analysis dimension
     * 
     * In an entity set, is always false.
     */
    bool after_analysis_dim;

    /**
     * The attribute for the dimension (reference to pointer)
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol*& attribute;

    /**
     * The attribute for the dimension
     * 
     * Only valid after post-parse.
     */
    AgentVarSymbol* pp_attribute;

    /**
     * The enumeration for the dimension
     * 
     * Only valid after post-parse.
     */
    EnumerationSymbol* pp_enumeration;
};

