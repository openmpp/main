/**
* @file    EntityTableMeasureSymbol.h
* Declarations for the EntityTableMeasureSymbol class.
*/
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <string>
#include "TableMeasureSymbol.h"
#include "Symbol.h"

using namespace std;

/**
* EntityTableMeasureSymbol.
*/
class EntityTableMeasureSymbol : public TableMeasureSymbol
{
private:
    typedef TableMeasureSymbol super;

public:
    bool is_base_symbol() const { return false; }

    EntityTableMeasureSymbol(Symbol *table, ExprForTable *root, int index, yy::location decl_loc = yy::location())
        : TableMeasureSymbol(table, symbol_name(table, index), index, decl_loc)
        , root(root)
    {
    }

    void post_parse(int pass);

    /**
     * Perform post-parse operations on nodes of a ExprForTable tree
     * 
     * Assigns the pp_accumulator pointer using the assumulator member from the parse phase.
     *
     * @param [in,out] node The root of the expression tree.
     *
     * ### return Result as a \a CodeBlock.
     */
    void post_parse_traverse(ExprForTable *node);

    enum expression_style {
        cxx,
        sql
    };

    /**
     * Get C++ expression using accumulators.
     *
     * @param node  The root of the expression tree.
     * @param style The style.
     *
     * @return Result as a \a CodeBlock.
     */
    string get_expression(const ExprForTable *node, expression_style style);

    /**
     * Root of the expression tree.
     */
    ExprForTable *root;
};

