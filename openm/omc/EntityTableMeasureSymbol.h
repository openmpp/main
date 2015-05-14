/**
* @file    EntityTableMeasureSymbol.h
* Declarations for the EntityTableMeasureSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include "Symbol.h"

using namespace std;

/**
* EntityTableMeasureSymbol.
*/
class EntityTableMeasureSymbol : public Symbol
{
private:
    typedef Symbol super;

public:
    bool is_base_symbol() const { return false; }

    EntityTableMeasureSymbol(Symbol *table, ExprForTable *root, int index, yy::location decl_loc = yy::location())
        : Symbol(symbol_name(table, index), decl_loc)
        , root(root)
        , index(index)
        , table(table->stable_rp())
        , pp_table(nullptr)
    {
    }

    // Construct symbol name for the table expression symbol.
    // Example: BasicDemography.Expr0
    static string symbol_name(const Symbol* table, int index);

    void post_parse(int pass);

    void post_parse_traverse(ExprForTable *node);

    enum expression_style {
        cxx,
        sql
    };
    string get_expression(const ExprForTable *node, expression_style style);

    /**
    * Root of the expression tree
    */

    ExprForTable *root;

    /**
    * Zero-based index of the expression in the expression table dimension
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

    EntityTableSymbol* pp_table;
};

