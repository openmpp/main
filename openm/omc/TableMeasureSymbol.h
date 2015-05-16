/**
* @file    TableMeasureSymbol.h
* Declarations for the TableMeasureSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include "Symbol.h"

using namespace std;

/**
* TableMeasureSymbol.
*
*/
class TableMeasureSymbol : public Symbol
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
    TableMeasureSymbol(Symbol *table, string measure_name, int index, yy::location decl_loc = yy::location())
        : Symbol(symbol_name(table, index), decl_loc)
        , measure_name(measure_name)
        , index(index)
        , table(table->stable_rp())
        , pp_table(nullptr)
    {
    }

    // Construct symbol name for the table measure symbol.
    // Example: BasicDemography.Expr0
    static string symbol_name(const Symbol* table, int index);

    void post_parse(int pass);

    string pretty_name() const;

    /**
     * Name of the measure
     */
    string measure_name;

    /**
     * Zero-based index of the measure in the table measures.
     */
    int index;

    /**
     * The table containing this measure (reference to pointer)
     * 
     * Stable to symbol morphing during parse phase.
     */
    Symbol*& table;

    /**
     * The table containing this measure (pointer)
     * 
     * Only valid after post-parse phase 1.
     */
    TableSymbol* pp_table;
};
