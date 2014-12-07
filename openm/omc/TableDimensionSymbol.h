/**
* @file    TableDimensionSymbol.h
* Declarations for the TableDimensionSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include "Symbol.h"

using namespace std;

/**
* TableDimensionSymbol.
* 
* Corresponds to a dimension of a table (excluding the expression dimension of hte table).
*/
class TableDimensionSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    TableDimensionSymbol(Symbol *table, int index, yy::location decl_loc = yy::location())
        : Symbol(symbol_name(table, index), decl_loc)
        , index(index)
        , table(table->stable_rp())
        , pp_table(nullptr)
    {
    }

    // Construct symbol name for the table expression symbol.
    // Example: BasicDemography.Dim0
    static string symbol_name(const Symbol* table, int index);

    void post_parse(int pass);

    /**
     * Zero-based index of the dimension in the table.
     */
    int index;

    /**
     * The table containing this expression (reference to pointer)
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol*& table;

    /**
     * The table containing this expression (pointer)
     * 
     * Only valid after post-parse phase 1.
     */
    TableSymbol* pp_table;
};

