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

    DimensionSymbol(Symbol *table, int index, Symbol *attribute, bool has_margin, yy::location decl_loc = yy::location())
        : Symbol(symbol_name(table, index), decl_loc)
        , index(index)
        , has_margin(has_margin)
        , attribute(attribute->stable_rp())
        , pp_attribute(nullptr)
        , table(table->stable_rp())
        , pp_table(nullptr)
    {
    }

    // Construct symbol name for the table expression symbol.
    // Example: BasicDemography.Dim0
    static string symbol_name(const Symbol* table, int index);

    void post_parse(int pass);

    /**
     * true if this dimension has a margin (using '+')
     */
    bool has_margin;

    /**
     * Zero-based index of the dimension in the table.
     */
    int index;

    /**
     * The attribute for the dimension (reference to pointer)
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol*& attribute;

    /**
     * The attribute for the dimension (pointer)
     * 
     * Only valid after post-parse.
     */
    AgentVarSymbol* pp_attribute;

    /**
     * The containing table (reference to pointer)
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol*& table;

    /**
     * The containing table (pointer)
     * 
     * Only valid after post-parse.
     */
    TableSymbol* pp_table;
};

