/**
* @file    TableExpressionSymbol.h
* Definitions for the TableExpressionSymbol class.
*/
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <cassert>
#include <algorithm>
#include <typeinfo>
#include <set>
#include "ast.h"
#include "token.h"

// static
string TableExpressionSymbol::symbol_name(const Symbol* table, int index)
{
    assert(table);
    return table->name + ".Expr" + to_string(index);
}

/**
* Post-parse operations for TableExpressionSymbol
*/

void TableExpressionSymbol::post_parse(int pass)
{
    // First perform post-parse operations at next level up in Symbol hierarchy
    super::post_parse(pass);

    switch (pass) {
    case 1:
        // assign direct pointer to table for post-parse use
        pp_table = dynamic_cast<TableSymbol *> (table);
        // Add this table expression to the table's list of expressions
        pp_table->pp_expressions.push_back(this);
        // Perform post-parse operation to each element of the expression
        post_parse_traverse(root);

        break;
    default:
        break;
    }
}

/**
* Perform post-parse operations on nodes of a ExprForTable tree
*
* @param   node    The root of the expression tree.
*
* @return  Result as a \a CodeBlock
*/

void TableExpressionSymbol::post_parse_traverse(ExprForTable *node)
{
    auto leaf = dynamic_cast<ExprForTableLeaf *>(node);
    if (leaf != nullptr) {
        (leaf->pp_accumulator) = dynamic_cast<TableAccumulatorSymbol *>(leaf->accumulator);
    }
    else {
        auto op = dynamic_cast<ExprForTableOp *>(node);
        assert(op);
        post_parse_traverse(op->left);
        post_parse_traverse(op->right);
    }
}

/**
* Get C++ expression using accumulators
*
* @param   node    The root of the expression tree.
*
* @return  Result as a \a CodeBlock
*/

string TableExpressionSymbol::get_expression(const ExprForTable *node, expression_style style)
{
    const ExprForTableLeaf* leaf = dynamic_cast<const ExprForTableLeaf *>(node);
    if (leaf != nullptr) {
        const TableAccumulatorSymbol *accumulator = leaf->pp_accumulator;
        string result;
        switch (style) {
        case cxx:
            result = "accumulators[" + to_string(accumulator->index) + "][cell]";
            break;
        case sql:
        {
                    string agg_func = "";
                    switch (accumulator->accumulator) {
                    case token::TK_sum:
                        agg_func = "OM_SUM";
                        break;
                    case token::TK_min:
                        agg_func = "OM_MIN";
                        break;
                    case token::TK_max:
                        agg_func = "OM_MAX";
                        break;
                    default:
                        assert(0);
                    }
                    result = agg_func + "( acc" + to_string(accumulator->index) + " )";
                    break;
        }
        default:
            assert(0);
        }
        return result;
    }
    else {
        auto binary_node = dynamic_cast<const ExprForTableOp *>(node);
        assert(binary_node);
        string expr_left = get_expression(binary_node->left, style);
        string expr_right = get_expression(binary_node->right, style);
        return "( " + expr_left + " " + token_to_string(binary_node->op) + " " + expr_right + " )";
    }
}
